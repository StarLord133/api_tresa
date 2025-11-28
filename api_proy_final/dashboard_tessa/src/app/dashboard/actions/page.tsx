"use client"

import * as React from "react"
import { AppSidebar } from "@/components/app-sidebar"
import { SiteHeader } from "@/components/site-header"
import {
    SidebarInset,
    SidebarProvider,
} from "@/components/ui/sidebar"
import { Button } from "@/components/ui/button"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { IconBulb } from "@tabler/icons-react"
import { toast } from "sonner"

export default function ActionsPage() {
    const [ledState, setLedState] = React.useState(false)
    const [loading, setLoading] = React.useState(false)

    // Fetch initial state
    React.useEffect(() => {
        fetch('https://api-tresa.onrender.com/api/led')
            .then(res => res.json())
            .then(data => setLedState(data.led))
            .catch(err => console.error("Error fetching LED state:", err))
    }, [])

    const toggleLed = async () => {
        setLoading(true)
        const newState = !ledState
        try {
            const response = await fetch('https://api-tresa.onrender.com/api/led', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ state: newState }),
            })

            if (response.ok) {
                setLedState(newState)
                toast.success(`LED ${newState ? 'encendido' : 'apagado'} correctamente`)
            } else {
                toast.error("Error al cambiar estado del LED")
            }
        } catch (error) {
            console.error("Error toggling LED:", error)
            toast.error("Error de conexión")
        } finally {
            setLoading(false)
        }
    }

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
                            <h1 className="text-2xl font-bold">Acciones</h1>

                            <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-3">
                                <Card>
                                    <CardHeader>
                                        <CardTitle className="flex items-center gap-2">
                                            <IconBulb className={ledState ? "text-yellow-500 fill-yellow-500" : "text-muted-foreground"} />
                                            Control de LED
                                        </CardTitle>
                                        <CardDescription>Encender o apagar el LED del dispositivo</CardDescription>
                                    </CardHeader>
                                    <CardContent>
                                        <Button
                                            onClick={toggleLed}
                                            disabled={loading}
                                            className="w-full"
                                            variant={ledState ? "default" : "outline"}
                                        >
                                            {loading ? "Procesando..." : (ledState ? "Apagar LED" : "Encender LED")}
                                        </Button>
                                    </CardContent>
                                </Card>
                            </div>

                        </div>
                    </div>
                </div>
            </SidebarInset>
        </SidebarProvider>
    )
}
