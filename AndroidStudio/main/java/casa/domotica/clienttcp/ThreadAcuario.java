package casa.domotica.clienttcp;

import android.app.ActivityManager;
import android.graphics.Color;
import android.widget.Button;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

class ThreadAcuario extends Thread{
    String comando;
    AcuarioActivity acuarioActivity;
    Button buttonTiraLed,buttonLed;
    PrintWriter writer;
    Socket s;
    String incomeMessage;
    public ThreadAcuario(AcuarioActivity acuarioActivity, Button buttonTiraLed, Button buttonLed,String comando) {
        this.acuarioActivity = acuarioActivity;
        this.buttonTiraLed = buttonTiraLed;
        this.buttonLed = buttonLed;
        this.comando = comando;
    }
    @Override
    public void run() {
        try {
        s = new Socket("domosapienstfg.ddns.net",1996);
        BufferedReader reader =new BufferedReader(new InputStreamReader(s.getInputStream()));
        writer = new PrintWriter(s.getOutputStream());
        writer.write(this.comando + '\n');
        writer.flush();
        String estado = null;
        incomeMessage = reader.readLine();
            if(incomeMessage!=null) {
               if(incomeMessage.indexOf("</response>") >= 0){
                    int posFin = incomeMessage.indexOf("</response>");
                    int posIni = incomeMessage.indexOf("<response>");
                    String response = incomeMessage.substring(posIni+10,posFin);
                    if(response.indexOf("</ack>") >= 0){
                        posFin = response.indexOf("</ack>");
                        posIni = response.indexOf("<ack>");
                        String ack = response.substring(posIni+5,posFin);
                        if(Integer.parseInt(ack) == 1){
                            if(response.indexOf("</estado>") >= 0){
                                posFin = response.indexOf("</estado>");
                                posIni = response.indexOf("<estado>");
                                estado = response.substring(posIni+8,posFin);
                            }
                        }
                    }
                    else{
                        Toast.makeText(this.acuarioActivity, "Ha habido un error. ACK=0", Toast.LENGTH_SHORT).show();
                    }
               }
               if(estado!=null) {
                   if (estado.equals("Leds Encendidos")) {
                       this.buttonTiraLed.setTextColor(Color.RED);
                       this.buttonLed.setTextColor(Color.GREEN);
                   } else if (estado.equals("Tira Encendida")) {
                       this.buttonLed.setTextColor(Color.RED);
                       this.buttonTiraLed.setTextColor(Color.GREEN);
                   } else if (estado.equals("Los dos Encendidos")) {
                       this.buttonTiraLed.setTextColor(Color.GREEN);
                       this.buttonLed.setTextColor(Color.GREEN);
                   } else if (estado.equals("Los dos Apagados")) {
                       this.buttonLed.setTextColor(Color.RED);
                       this.buttonTiraLed.setTextColor(Color.RED);
                   } else if (estado.equals("Error en evaluación luces")) {
                       Toast toast = Toast.makeText(this.acuarioActivity, "Respuesta del servidor frontal-->" + estado, Toast.LENGTH_LONG);
                       toast.show();
                       this.buttonLed.setTextColor(Color.BLACK);
                       this.buttonTiraLed.setTextColor(Color.BLACK);
                   } else if (estado.equals("Error connexión acuario")) {
                       Toast toast = Toast.makeText(this.acuarioActivity, "Respuesta del servidor frontal-->" + estado, Toast.LENGTH_LONG);
                       toast.show();
                       this.buttonLed.setTextColor(Color.BLACK);
                       this.buttonTiraLed.setTextColor(Color.BLACK);
                   }
               }
               else{
                   Toast.makeText(this.acuarioActivity, "Respuesta erronea del servidor Frontal", Toast.LENGTH_SHORT).show();
               }
            }
        writer.close();
            s.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
