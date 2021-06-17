package casa.domotica.clienttcp;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.os.AsyncTask;
import android.widget.Button;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

class BackgroundTaskEstadoRiego extends AsyncTask<String, Void, String> {
    public RiegoActivity superActivity;
    String message, incomeMessage;
    Socket s;
    PrintWriter writer;
    Boolean iniciarPrograma = true;

    public BackgroundTaskEstadoRiego(RiegoActivity superActivity) {
        this.superActivity = superActivity;

    }

    @SuppressLint("WrongThread")
    @Override
    protected String doInBackground(String... params) {
        superActivity.iniciarProgramaRiego = false;
        message = params[0];
        String estado = null;
        while (iniciarPrograma) {
            try {
                s = new Socket("domosapienstfg.ddns.net", 1996);
                BufferedReader reader = new BufferedReader(new InputStreamReader(s.getInputStream()));
                writer = new PrintWriter(s.getOutputStream());
                writer.write(message + '\n');
                writer.flush();
                incomeMessage = reader.readLine();
                writer.close();
                s.close();
                Thread.sleep(5000);
            } catch (Exception e) {
                return "Error";
            }
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
                    Toast.makeText(this.superActivity, "Ha habido un error. ACK=0", Toast.LENGTH_SHORT).show();
                }
            }
            if (estado != null) {
                return estado;
            }
            else{
                Toast.makeText(this.superActivity, "No encuentro estado en la respuesta.", Toast.LENGTH_SHORT).show();
            }
        }
        return "Riego Apagado";
    }

    @Override
    protected void onPostExecute(String incomeMessage) {
        if (incomeMessage.equals("Riego Apagado")) {
            iniciarPrograma = false;
            superActivity.iniciarZ1.setTextColor(Color.RED);
            superActivity.iniciarZ2.setTextColor(Color.RED);
            superActivity.iniciarPrograma.setTextColor(Color.RED);
        } else if (incomeMessage.equals("Programación Lado Izquierdo")) {
            superActivity.iniciarZ1.setTextColor(Color.GREEN);
            superActivity.iniciarZ2.setTextColor(Color.RED);
        } else if (incomeMessage.equals("Programación Lado Derecho")) {
            superActivity.iniciarZ1.setTextColor(Color.RED);
            superActivity.iniciarZ2.setTextColor(Color.GREEN);
        }else if (incomeMessage.equals("Error")) {
            Toast.makeText(superActivity, "Error Estado Riego, middleWare ha caido", Toast.LENGTH_SHORT).show();
        } else if (incomeMessage.equals("Riego Apagado")) {
            Toast.makeText(superActivity, "Riego ha acabado", Toast.LENGTH_SHORT).show();
            superActivity.iniciarProgramaRiego = true;
        }
    }
}