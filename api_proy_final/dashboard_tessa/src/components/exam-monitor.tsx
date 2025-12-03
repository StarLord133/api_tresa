import { useState, useEffect } from 'react';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Play, Square, Camera, AlertTriangle, CheckCircle2, Eye, QrCode } from 'lucide-react';

const API_URL = import.meta.env.VITE_API_URL || 'https://api-tresa.onrender.com';

interface ExamAlert {
    id: number;
    timestamp: string;
    incident_number: number;
    image_url: string;
    detections: Array<{
        object: string;
        confidence: number;
    }>;
    severity: 'high' | 'medium' | 'low';
    reviewed: boolean;
}

interface ExamStatus {
    active: boolean;
    start_time?: string;
    elapsed_seconds?: number;
    incident_count: number;
}

export function ExamMonitor() {
    const [examStatus, setExamStatus] = useState<ExamStatus>({ active: false, incident_count: 0 });
    const [alerts, setAlerts] = useState<ExamAlert[]>([]);
    const [snapshot, setSnapshot] = useState<string | null>(null);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState<string | null>(null);
    const [attendanceActive, setAttendanceActive] = useState(false);

    // Polling para obtener estado del examen
    useEffect(() => {
        const fetchStatus = async () => {
            try {
                const response = await fetch(`${API_URL}/api/exam/status`);
                const data = await response.json();
                setExamStatus(data);
            } catch (err) {
                console.error('Error fetching exam status:', err);
            }
        };

        fetchStatus();
        const interval = setInterval(fetchStatus, 2000); // Cada 2 segundos

        return () => clearInterval(interval);
    }, []);

    // Obtener alertas
    useEffect(() => {
        const fetchAlerts = async () => {
            try {
                const response = await fetch(`${API_URL}/api/exam-alerts?limit=20`);
                const data = await response.json();
                setAlerts(data);
            } catch (err) {
                console.error('Error fetching alerts:', err);
            }
        };

        fetchAlerts();
        const interval = setInterval(fetchAlerts, 5000); // Cada 5 segundos

        return () => clearInterval(interval);
    }, []);

    const startExam = async () => {
        setLoading(true);
        setError(null);
        try {
            const response = await fetch(`${API_URL}/api/exam/start`, {
                method: 'POST',
            });

            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.error || 'Failed to start exam');
            }

            const data = await response.json();
            setExamStatus({ active: true, start_time: data.start_time, incident_count: 0 });
        } catch (err: any) {
            setError(err.message);
        } finally {
            setLoading(false);
        }
    };

    const stopExam = async () => {
        setLoading(true);
        setError(null);
        try {
            const response = await fetch(`${API_URL}/api/exam/stop`, {
                method: 'POST',
            });

            if (!response.ok) {
                throw new Error('Failed to stop exam');
            }

            const data = await response.json();
            setExamStatus({ active: false, incident_count: data.incident_count });
        } catch (err: any) {
            setError(err.message);
        } finally {
            setLoading(false);
        }
    };

    const takeSnapshot = async () => {
        try {
            const response = await fetch(`${API_URL}/api/exam/snapshot`);
            const data = await response.json();
            setSnapshot(data.image);
        } catch (err) {
            console.error('Error taking snapshot:', err);
        }
    };

    const markAsReviewed = async (id: number) => {
        try {
            await fetch(`${API_URL}/api/exam-alerts/${id}/review`, {
                method: 'PATCH',
            });
            setAlerts(alerts.map(alert =>
                alert.id === id ? { ...alert, reviewed: true } : alert
            ));
        } catch (err) {
            console.error('Error marking as reviewed:', err);
        }
    };

    const toggleAttendance = async () => {
        try {
            const endpoint = attendanceActive ? '/api/attendance/stop' : '/api/attendance/start';
            const response = await fetch(`${API_URL}${endpoint}`, { method: 'POST' });
            if (response.ok) {
                setAttendanceActive(!attendanceActive);
            }
        } catch (err) {
            console.error('Error toggling attendance:', err);
        }
    };

    const formatElapsedTime = (seconds: number) => {
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const secs = Math.floor(seconds % 60);
        return `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    };

    const getSeverityColor = (severity: string) => {
        switch (severity) {
            case 'high': return 'destructive';
            case 'medium': return 'default';
            case 'low': return 'secondary';
            default: return 'default';
        }
    };

    return (
        <div className="space-y-6">
            {/* Control Panel */}
            <Card>
                <CardHeader>
                    <CardTitle className="flex items-center gap-2">
                        <Camera className="h-5 w-5" />
                        Control de Modo Examen
                    </CardTitle>
                    <CardDescription>
                        Sistema de vigilancia anti-trampa con detección de objetos
                    </CardDescription>
                </CardHeader>
                <CardContent className="space-y-4">
                    {error && (
                        <Alert variant="destructive">
                            <AlertTriangle className="h-4 w-4" />
                            <AlertDescription>{error}</AlertDescription>
                        </Alert>
                    )}

                    <div className="flex items-center justify-between">
                        <div className="space-y-1">
                            <div className="flex items-center gap-2">
                                <span className="text-sm font-medium">Estado:</span>
                                <Badge variant={examStatus.active ? 'default' : 'secondary'}>
                                    {examStatus.active ? '🟢 Activo' : '⚪ Inactivo'}
                                </Badge>
                            </div>

                            {examStatus.active && examStatus.elapsed_seconds !== undefined && (
                                <div className="text-sm text-muted-foreground">
                                    Tiempo transcurrido: {formatElapsedTime(examStatus.elapsed_seconds)}
                                </div>
                            )}

                            <div className="text-sm text-muted-foreground">
                                Incidentes detectados: <span className="font-semibold">{examStatus.incident_count}</span>
                            </div>
                        </div>

                        <div className="flex gap-2">
                            {!examStatus.active ? (
                                <Button onClick={startExam} disabled={loading}>
                                    <Play className="mr-2 h-4 w-4" />
                                    Iniciar Examen
                                </Button>
                            ) : (
                                <Button onClick={stopExam} disabled={loading} variant="destructive">
                                    <Square className="mr-2 h-4 w-4" />
                                    Finalizar Examen
                                </Button>
                            )}

                            <Button onClick={takeSnapshot} variant="outline">
                                <Camera className="mr-2 h-4 w-4" />
                                Snapshot
                            </Button>
                        </div>

                        <div className="flex gap-2 mt-4 pt-4 border-t">
                            <Button
                                onClick={toggleAttendance}
                                variant={attendanceActive ? "destructive" : "secondary"}
                                className="w-full sm:w-auto"
                            >
                                <QrCode className="mr-2 h-4 w-4" />
                                {attendanceActive ? "Terminar Pase de Lista" : "Pase de Lista (QR)"}
                            </Button>
                        </div>
                    </div>

                    {snapshot && (
                        <div className="mt-4">
                            <p className="text-sm font-medium mb-2">Vista previa de cámara:</p>
                            <img src={snapshot} alt="Camera snapshot" className="rounded-lg border max-w-md" />
                        </div>
                    )}
                </CardContent>
            </Card>

            {/* Alerts List */}
            <Card>
                <CardHeader>
                    <CardTitle className="flex items-center gap-2">
                        <AlertTriangle className="h-5 w-5" />
                        Alertas de Incidentes ({alerts.length})
                    </CardTitle>
                    <CardDescription>
                        Objetos prohibidos detectados durante los exámenes
                    </CardDescription>
                </CardHeader>
                <CardContent>
                    {alerts.length === 0 ? (
                        <div className="text-center py-8 text-muted-foreground">
                            <CheckCircle2 className="h-12 w-12 mx-auto mb-2 opacity-50" />
                            <p>No hay incidentes registrados</p>
                        </div>
                    ) : (
                        <div className="space-y-4">
                            {alerts.map((alert) => (
                                <div
                                    key={alert.id}
                                    className={`border rounded-lg p-4 ${alert.reviewed ? 'opacity-60' : ''
                                        }`}
                                >
                                    <div className="flex items-start justify-between mb-3">
                                        <div>
                                            <div className="flex items-center gap-2 mb-1">
                                                <span className="font-semibold">Incidente #{alert.incident_number}</span>
                                                <Badge variant={getSeverityColor(alert.severity)}>
                                                    {alert.severity}
                                                </Badge>
                                                {alert.reviewed && (
                                                    <Badge variant="outline">
                                                        <CheckCircle2 className="h-3 w-3 mr-1" />
                                                        Revisado
                                                    </Badge>
                                                )}
                                            </div>
                                            <p className="text-sm text-muted-foreground">
                                                {new Date(alert.timestamp).toLocaleString()}
                                            </p>
                                        </div>

                                        {!alert.reviewed && (
                                            <Button
                                                size="sm"
                                                variant="outline"
                                                onClick={() => markAsReviewed(alert.id)}
                                            >
                                                <Eye className="h-4 w-4 mr-1" />
                                                Marcar revisado
                                            </Button>
                                        )}
                                    </div>

                                    <div className="grid md:grid-cols-2 gap-4">
                                        <div>
                                            <p className="text-sm font-medium mb-2">Objetos detectados:</p>
                                            <ul className="space-y-1">
                                                {alert.detections.map((detection, idx) => (
                                                    <li key={idx} className="text-sm flex items-center gap-2">
                                                        <span className="text-red-500">⚠️</span>
                                                        <span className="font-medium">{detection.object}</span>
                                                        <span className="text-muted-foreground">
                                                            ({(detection.confidence * 100).toFixed(0)}%)
                                                        </span>
                                                    </li>
                                                ))}
                                            </ul>
                                        </div>

                                        {alert.image_url && (
                                            <div>
                                                <p className="text-sm font-medium mb-2">Evidencia:</p>
                                                <img
                                                    src={alert.image_url}
                                                    alt={`Incident ${alert.incident_number}`}
                                                    className="rounded-lg border max-w-full h-auto"
                                                />
                                            </div>
                                        )}
                                    </div>
                                </div>
                            ))}
                        </div>
                    )}
                </CardContent>
            </Card>
        </div>
    );
}
