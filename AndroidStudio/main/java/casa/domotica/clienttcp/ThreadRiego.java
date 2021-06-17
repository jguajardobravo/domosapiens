package casa.domotica.clienttcp;

import android.graphics.Color;
import android.widget.Button;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

class ThreadRiego extends Thread{
    RiegoActivity riegoActivity;
    Button iniciarZ1,iniciarZ2,iniciarPrograma;
    String comando,incomeMessage;
    Socket s;
    PrintWriter writer;

    public ThreadRiego(RiegoActivity riegoActivity, Button iniciarZ1, Button iniciarZ2, Button iniciarPrograma, String comando) {
        this.comando = comando;
        this.iniciarPrograma = iniciarPrograma;
        this.iniciarZ1 = iniciarZ1;
        this.iniciarZ2 = iniciarZ2;
    }

    @Override
    public void run() {
        try {
            String estado = null;
            s = new Socket("domosapienstfg.ddns.net", 1996);
            BufferedReader reader = new BufferedReader(new InputStreamReader(s.getInputStream()));
            writer = new PrintWriter(s.getOutputStream());
            writer.write(this.comando + '\n');
            writer.flush();
            incomeMessage = reader.readLine();
            if (incomeMessage != null) {
                if (incomeMessage.contains("</response>")) {
                    int posFin = incomeMessage.indexOf("</response>");
                    int posIni = incomeMessage.indexOf("<response>");
                    String response = incomeMessage.substring(posIni+10, posFin);
                    if (response.contains("</ack>")) {
                        posFin = response.indexOf("</ack>");
                        posIni = response.indexOf("<ack>");
                        String ack = response.substring(posIni+5, posFin);
                        if (Integer.parseInt(ack) == 1) {
                            if (response.contains("</estado>")) {
                                posFin = response.indexOf("</estado>");
                                posIni = response.indexOf("<estado>");
                                estado = response.substring(posIni+8, posFin);
                            }
                        }
                    } else {
                        Toast.makeText(this.riegoActivity, "Ha habido un error. ACK=0", Toast.LENGTH_SHORT).show();
                    }
                }
            }
            if(estado != null){
                if (estado.equals("Riego Apagado")) {
                    this.iniciarPrograma.setTextColor(Color.RED);
                    this.iniciarZ1.setTextColor(Color.RED);
                    this.iniciarZ2.setTextColor(Color.RED);
                } else if (estado.equals("Lado Izquierdo riego Encendido")) {
                    this.iniciarPrograma.setTextColor(Color.RED);
                    this.iniciarZ1.setTextColor(Color.GREEN);
                    this.iniciarZ2.setTextColor(Color.RED);
                } else if (estado.equals("Lado Derecho riego Encendido")) {
                    this.iniciarPrograma.setTextColor(Color.RED);
                    this.iniciarZ1.setTextColor(Color.RED);
                    this.iniciarZ2.setTextColor(Color.GREEN);
                } else if (estado.equals("Programación Lado Derecho")) {
                    this.iniciarZ1.setTextColor(Color.RED);
                    this.iniciarPrograma.setTextColor(Color.GREEN);
                    this.iniciarZ2.setTextColor(Color.GREEN);
                } else if (estado.equals("Programación Lado Izquierdo")) {
                    this.iniciarZ1.setTextColor(Color.GREEN);
                    this.iniciarZ2.setTextColor(Color.RED);
                    this.iniciarPrograma.setTextColor(Color.GREEN);
                }
            }
            else{
                Toast.makeText(this.riegoActivity, "Respuesta erronea del servidor Frontal", Toast.LENGTH_SHORT).show();
            }
            writer.close();
            s.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
