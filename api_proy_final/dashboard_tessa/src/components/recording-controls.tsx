import { useState, useEffect } from 'react';
import { Button } from "@/components/ui/button";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Mic, Square } from "lucide-react";

export function RecordingControls() {
    const [isRecording, setIsRecording] = useState(false);
    const [loading, setLoading] = useState(false);

    // Poll status periodically to sync with other clients or ESP32
    useEffect(() => {
        const checkStatus = async () => {
            try {
                const res = await fetch('https://api-tresa.onrender.com/api/recording/status');
                const data = await res.json();
                setIsRecording(data.recording);
            } catch (error) {
                console.error("Error checking recording status:", error);
            }
        };

        checkStatus();
        const interval = setInterval(checkStatus, 2000);
        return () => clearInterval(interval);
    }, []);

    const toggleRecording = async () => {
        setLoading(true);
        const endpoint = isRecording ? '/api/recording/stop' : '/api/recording/start';
        try {
            const res = await fetch(`https://api-tresa.onrender.com${endpoint}`, {
                method: 'POST',
            });
            const data = await res.json();
            if (data.recording !== undefined) {
                setIsRecording(data.recording);
            }
        } catch (error) {
            console.error("Error toggling recording:", error);
        } finally {
            setLoading(false);
        }
    };

    return (
        <Card>
            <CardHeader>
                <CardTitle>Control de Grabación</CardTitle>
            </CardHeader>
            <CardContent className="flex flex-col items-center gap-4">
                <div className={`w-4 h-4 rounded-full ${isRecording ? 'bg-red-500 animate-pulse' : 'bg-gray-300'}`} />
                <p className="text-sm text-muted-foreground">
                    {isRecording ? 'Grabando...' : 'Detenido'}
                </p>
                <Button
                    variant={isRecording ? "destructive" : "default"}
                    onClick={toggleRecording}
                    disabled={loading}
                    className="w-full"
                >
                    {isRecording ? (
                        <>
                            <Square className="mr-2 h-4 w-4" /> Detener Grabación
                        </>
                    ) : (
                        <>
                            <Mic className="mr-2 h-4 w-4" /> Iniciar Grabación
                        </>
                    )}
                </Button>
            </CardContent>
        </Card>
    );
}
