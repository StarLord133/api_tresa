"use client"

import * as React from "react"
import {
    type ColumnDef,
    flexRender,
    getCoreRowModel,
    getPaginationRowModel,
    getSortedRowModel,
    type SortingState,
    useReactTable,
} from "@tanstack/react-table"
import {
    Table,
    TableBody,
    TableCell,
    TableHead,
    TableHeader,
    TableRow,
} from "@/components/ui/table"
import { Button } from "@/components/ui/button"
import { IconChevronLeft, IconChevronRight, IconPlayerPlay } from "@tabler/icons-react"

export interface Recording {
    id: number
    fecha: string
    archivo_url: string
    transcripcion: string
}

const columns: ColumnDef<Recording>[] = [
    {
        accessorKey: "id",
        header: "ID",
    },
    {
        accessorKey: "fecha",
        header: "Fecha",
        cell: ({ row }) => {
            return new Date(row.original.fecha).toLocaleString()
        }
    },
    {
        accessorKey: "transcripcion",
        header: "Transcripción",
    },
    {
        id: "actions",
        header: "Audio",
        cell: ({ row }) => {
            const playAudio = () => {
                // Asumimos que el servidor Python corre en el puerto 5001 y es accesible
                // Ajusta la URL base si es necesario (e.g., si usas Render)
                const baseUrl = "https://api-tresa-python.onrender.com"; // CAMBIAR POR TU URL DE PYTHON
                // O si es local: "http://localhost:5001"

                // Si la URL ya viene completa, úsala. Si es relativa, concatena.
                let url = row.original.archivo_url;
                if (url.startsWith("/")) {
                    url = baseUrl + url;
                }

                const audio = new Audio(url);
                audio.play().catch(e => alert("Error reproduciendo audio: " + e.message));
            }

            return (
                <Button variant="ghost" size="sm" onClick={playAudio}>
                    <IconPlayerPlay className="h-4 w-4 mr-2" />
                    Reproducir
                </Button>
            )
        },
    },
]

export function RecordingsTable() {
    const [data, setData] = React.useState<Recording[]>([])
    const [sorting, setSorting] = React.useState<SortingState>([])
    const [pagination, setPagination] = React.useState({
        pageIndex: 0,
        pageSize: 5,
    })

    React.useEffect(() => {
        const fetchData = async () => {
            try {
                const response = await fetch('https://api-tresa.onrender.com/api/recordings')
                if (response.ok) {
                    const jsonData = await response.json()
                    setData(jsonData)
                }
            } catch (error) {
                console.error("Error fetching recordings:", error)
            }
        }

        fetchData()
        const interval = setInterval(fetchData, 10000) // Refrescar cada 10s
        return () => clearInterval(interval)
    }, [])

    const table = useReactTable({
        data,
        columns,
        getCoreRowModel: getCoreRowModel(),
        getPaginationRowModel: getPaginationRowModel(),
        getSortedRowModel: getSortedRowModel(),
        onSortingChange: setSorting,
        onPaginationChange: setPagination,
        state: {
            sorting,
            pagination,
        },
    })

    return (
        <div className="space-y-4">
            <div className="rounded-md border">
                <Table>
                    <TableHeader>
                        {table.getHeaderGroups().map((headerGroup) => (
                            <TableRow key={headerGroup.id}>
                                {headerGroup.headers.map((header) => {
                                    return (
                                        <TableHead key={header.id}>
                                            {header.isPlaceholder
                                                ? null
                                                : flexRender(
                                                    header.column.columnDef.header,
                                                    header.getContext()
                                                )}
                                        </TableHead>
                                    )
                                })}
                            </TableRow>
                        ))}
                    </TableHeader>
                    <TableBody>
                        {table.getRowModel().rows?.length ? (
                            table.getRowModel().rows.map((row) => (
                                <TableRow
                                    key={row.id}
                                    data-state={row.getIsSelected() && "selected"}
                                >
                                    {row.getVisibleCells().map((cell) => (
                                        <TableCell key={cell.id}>
                                            {flexRender(
                                                cell.column.columnDef.cell,
                                                cell.getContext()
                                            )}
                                        </TableCell>
                                    ))}
                                </TableRow>
                            ))
                        ) : (
                            <TableRow>
                                <TableCell
                                    colSpan={columns.length}
                                    className="h-24 text-center"
                                >
                                    No recordings found.
                                </TableCell>
                            </TableRow>
                        )}
                    </TableBody>
                </Table>
            </div>
            <div className="flex items-center justify-end space-x-2">
                <Button
                    variant="outline"
                    size="sm"
                    onClick={() => table.previousPage()}
                    disabled={!table.getCanPreviousPage()}
                >
                    <IconChevronLeft className="h-4 w-4" />
                    Previous
                </Button>
                <Button
                    variant="outline"
                    size="sm"
                    onClick={() => table.nextPage()}
                    disabled={!table.getCanNextPage()}
                >
                    Next
                    <IconChevronRight className="h-4 w-4" />
                </Button>
            </div>
        </div>
    )
}
