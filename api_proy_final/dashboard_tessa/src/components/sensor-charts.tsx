"use client"

import * as React from "react"
import { Area, AreaChart, CartesianGrid, XAxis, YAxis } from "recharts"
import {
    Card,
    CardContent,
    CardDescription,
    CardHeader,
    CardTitle,
} from "@/components/ui/card"
import {
    type ChartConfig,
    ChartContainer,
    ChartTooltip,
    ChartTooltipContent,
} from "@/components/ui/chart"

export interface SensorData {
    id: number
    temperatura: number
    humedad: number
    distancia: number

    fecha?: string
}

interface SensorChartsProps {
    data: SensorData[]
}

const tempConfig = {
    temperatura: {
        label: "Temperatura (°C)",
        color: "hsl(var(--chart-1))",
    },
} satisfies ChartConfig

const humConfig = {
    humedad: {
        label: "Humedad (%)",
        color: "hsl(var(--chart-2))",
    },
} satisfies ChartConfig

const distConfig = {
    distancia: {
        label: "Distancia (cm)",
        color: "hsl(var(--chart-3))",
    },
} satisfies ChartConfig

export function SensorCharts({ data }: SensorChartsProps) {
    // Reverse data to show oldest to newest if it comes sorted by ID DESC
    const chartData = React.useMemo(() => [...data].reverse(), [data])

    return (
        <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-3">
            <Card>
                <CardHeader>
                    <CardTitle>Temperatura</CardTitle>
                    <CardDescription>Historial de temperatura</CardDescription>
                </CardHeader>
                <CardContent>
                    <ChartContainer config={tempConfig}>
                        <AreaChart data={chartData}>
                            <CartesianGrid vertical={false} />
                            <XAxis
                                dataKey="id"
                                tickLine={false}
                                axisLine={false}
                                tickMargin={8}
                                hide
                            />
                            <YAxis tickLine={false} axisLine={false} tickMargin={8} width={40} />
                            <ChartTooltip content={<ChartTooltipContent indicator="line" />} />
                            <Area
                                dataKey="temperatura"
                                type="natural"
                                fill="var(--color-temperatura)"
                                fillOpacity={0.4}
                                stroke="var(--color-temperatura)"
                            />
                        </AreaChart>
                    </ChartContainer>
                </CardContent>
            </Card>

            <Card>
                <CardHeader>
                    <CardTitle>Humedad</CardTitle>
                    <CardDescription>Historial de humedad</CardDescription>
                </CardHeader>
                <CardContent>
                    <ChartContainer config={humConfig}>
                        <AreaChart data={chartData}>
                            <CartesianGrid vertical={false} />
                            <XAxis
                                dataKey="id"
                                tickLine={false}
                                axisLine={false}
                                tickMargin={8}
                                hide
                            />
                            <YAxis tickLine={false} axisLine={false} tickMargin={8} width={40} />
                            <ChartTooltip content={<ChartTooltipContent indicator="line" />} />
                            <Area
                                dataKey="humedad"
                                type="natural"
                                fill="var(--color-humedad)"
                                fillOpacity={0.4}
                                stroke="var(--color-humedad)"
                            />
                        </AreaChart>
                    </ChartContainer>
                </CardContent>
            </Card>

            <Card>
                <CardHeader>
                    <CardTitle>Distancia</CardTitle>
                    <CardDescription>Historial de distancia</CardDescription>
                </CardHeader>
                <CardContent>
                    <ChartContainer config={distConfig}>
                        <AreaChart data={chartData}>
                            <CartesianGrid vertical={false} />
                            <XAxis
                                dataKey="id"
                                tickLine={false}
                                axisLine={false}
                                tickMargin={8}
                                hide
                            />
                            <YAxis tickLine={false} axisLine={false} tickMargin={8} width={40} />
                            <ChartTooltip content={<ChartTooltipContent indicator="line" />} />
                            <Area
                                dataKey="distancia"
                                type="natural"
                                fill="var(--color-distancia)"
                                fillOpacity={0.4}
                                stroke="var(--color-distancia)"
                            />
                        </AreaChart>
                    </ChartContainer>
                </CardContent>
            </Card>
        </div>
    )
}
