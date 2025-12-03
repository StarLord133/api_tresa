"use client"

import * as React from "react"
import { AppSidebar } from "@/components/app-sidebar"
import { RecordingsTable } from "@/components/recordings-table"
import { AttendanceTable } from "@/components/attendance-table"

import { DataTable, type SensorData } from "@/components/data-table"
import { SiteHeader } from "@/components/site-header"
import {
    SidebarInset,
    SidebarProvider,
} from "@/components/ui/sidebar"

export default function Page() {
    const [data, setData] = React.useState<SensorData[]>([])
    const [loading, setLoading] = React.useState(true)

    React.useEffect(() => {
        const fetchData = async () => {
            try {
                // TODO: Change this URL to your Render backend URL if running in production
                // e.g., https://your-app.onrender.com/api/data
                const response = await fetch('https://api-tresa.onrender.com/api/data')
                if (!response.ok) {
                    throw new Error('Network response was not ok')
                }
                const jsonData = await response.json()
                setData(jsonData)
            } catch (error) {
                console.error("Error fetching data:", error)
            } finally {
                setLoading(false)
            }
        }

        fetchData()
        // Refresh every 5 seconds
        const interval = setInterval(fetchData, 5000)
        return () => clearInterval(interval)
    }, [])

    return (
        <SidebarProvider
            style={
                {
                    "--sidebar-width": "calc(var(--spacing) * 72)",
                    "--header-height": "calc(var(--spacing) * 12)",
                } as React.CSSProperties
            }
        >
            <AppSidebar variant="inset" />
            <SidebarInset>
                <SiteHeader />
                <div className="flex flex-1 flex-col">
                    <div className="@container/main flex flex-1 flex-col gap-2">
                        <div className="flex flex-col gap-4 py-4 md:gap-6 md:py-6 px-4 lg:px-6">
                            <h1 className="text-2xl font-bold">Registros de Sensores</h1>
                            {loading ? (
                                <div>Cargando datos...</div>
                            ) : (
                                <>
                                    <DataTable data={data} />
                                    <div className="mt-8">
                                        <h2 className="text-xl font-bold mb-4">Grabaciones de Voz</h2>
                                        <RecordingsTable />
                                    </div>
                                    <div className="mt-8">
                                        <AttendanceTable />
                                    </div>
                                </>
                            )}
                        </div>
                    </div>
                </div>
            </SidebarInset>
        </SidebarProvider>
    )
}
