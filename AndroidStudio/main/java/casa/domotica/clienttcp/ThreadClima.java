package casa.domotica.clienttcp;

import android.graphics.Color;
import android.graphics.Paint;
import android.os.Looper;
import android.widget.Button;
import android.widget.Toast;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GridLabelRenderer;
import com.jjoe64.graphview.LabelFormatter;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

class ThreadClima extends Thread{
    String comando;
    Clima climaActivity;
    GraphView graph,graph2;
    String leyenda=null;
    String temperatura=null;
    String humedad = null;
    Socket s;
    String incomeMessage;
    public ThreadClima(Clima climaActivity, GraphView graph,GraphView graph2, String comando) {
        this.climaActivity = climaActivity;
        this.graph = graph;
        this.graph2 = graph2;
        this.comando = comando;
    }
    @Override
    public void run() {
        Looper.prepare();
        String estado=datosGrafica(comando);
        dibujarGraficas(temperatura,humedad,leyenda);
    }
    protected String datosGrafica(String message) {
        String estado= null;

        String incomeMessage = null;
        Socket s;
        try {
            s = new Socket("domosapienstfg.ddns.net", 1996);
            BufferedReader reader = new BufferedReader(new InputStreamReader(s.getInputStream()));
            PrintWriter writer = new PrintWriter(s.getOutputStream());
            writer.write(message + '\n');
            writer.flush();
            incomeMessage = reader.readLine();
            writer.close();
            s.close();
        } catch (Exception e) {
            Toast.makeText(climaActivity, "No se puede abrir socket", Toast.LENGTH_SHORT).show();
        }
        if (incomeMessage == null) {
            Toast.makeText(climaActivity, "LLega mensaje vacío", Toast.LENGTH_SHORT).show();
        } else {
            if (incomeMessage.indexOf("</response>") >= 0) {
                int posFin = incomeMessage.indexOf("</response>");
                int posIni = incomeMessage.indexOf("<response>");
                String response = incomeMessage.substring(posIni + 10, posFin);
                if (response.indexOf("</ack>") >= 0) {
                    posFin = response.indexOf("</ack>");
                    posIni = response.indexOf("<ack>");
                    String ack = response.substring(posIni + 5, posFin);
                    if (Integer.parseInt(ack) == 1) {
                        if (response.indexOf("</estado>") >= 0) {
                            posFin = response.indexOf("</estado>");
                            posIni = response.indexOf("<estado>");
                            estado = response.substring(posIni + 8, posFin);
                            if (estado.contains("</leyenda>")){
                                posFin = estado.indexOf("</leyenda>");
                                posIni = estado.indexOf("<leyenda>");
                                leyenda = estado.substring(posIni+9,posFin);
                            }
                            if(estado.contains("</temperatura>")) {
                                posFin = estado.indexOf("</temperatura>");
                                posIni = estado.indexOf("<temperatura>");
                                temperatura = estado.substring(posIni + 13, posFin);
                            }
                            if(estado.contains("</humedad>")) {
                                posFin = estado.indexOf("</humedad>");
                                posIni = estado.indexOf("<humedad>");
                                humedad = estado.substring(posIni + 9, posFin);
                            }
                        }
                    }
                } else {
                    Toast.makeText(climaActivity, "Ha habido un error. ACK=0", Toast.LENGTH_SHORT).show();
                }
            }
        }
        if (estado == null) {
            Toast.makeText(climaActivity, "No encuentro estado en la respuesta", Toast.LENGTH_SHORT).show();
        }
        return estado;
    }
    protected void dibujarGraficas(String temperatura,String humedad, String leyenda){
        String xAxis = "",titulo="";
        if(comando.equals("Temperatura")){
            titulo = "Grafica Hora actual";
            xAxis = "(minutos)";
        }
        else if(comando == "TempAno"){
            titulo = "grafica Año actual";
            xAxis = "(Meses)";
        }
        else if(comando.equals("TempMes")){
            titulo = "grafica Mes actual";
            xAxis = "(Días)";
        }
        else if(comando.equals("TempDia")){
            titulo = "grafica Dia actual";
            xAxis = "(Horas)";
        }
        DataPoint[] dataTemperatura;
        DataPoint[] dataHumedad;
        if(temperatura != null) {
            String[] arrayOfDataTemperatura = temperatura.split(",");
            String [] arrayOfDataHumedad = humedad.split(",");
            String [] arrayOfLegends = leyenda.split(",");
            dataTemperatura = new DataPoint[arrayOfDataTemperatura.length];
            dataHumedad = new DataPoint[arrayOfDataHumedad.length];
            for (int i = 0; i < arrayOfDataTemperatura.length; i++) {
                int indice_actual = Integer.parseInt(arrayOfLegends[i]);
                DataPoint myDataPointTemperatura = new DataPoint(indice_actual, Double.parseDouble(arrayOfDataTemperatura[i]));
                DataPoint myDataPointHumedad = new DataPoint(indice_actual, Double.parseDouble(arrayOfDataHumedad[i]));
                dataTemperatura[i] = myDataPointTemperatura;
                dataHumedad[i] = myDataPointHumedad;
            }

            graph.removeAllSeries();
            GridLabelRenderer glr = graph.getGridLabelRenderer();
            GridLabelRenderer gridLabelRenderer = graph.getGridLabelRenderer();
            gridLabelRenderer.setGridStyle(GridLabelRenderer.GridStyle.HORIZONTAL);
            glr.resetStyles();
            glr.setHorizontalAxisTitle(xAxis);
            glr.setVerticalAxisTitle("Temperatura (Cº)");
            LineGraphSeries serie = new LineGraphSeries(dataTemperatura);
            serie.setTitle("Temperatura");
            serie.setDrawDataPoints(true);
            serie.setDrawAsPath(true);
            Paint paint = new Paint();
            paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth(2);
            serie.setCustomPaint(paint);
            graph.setTitle(titulo);
            graph.getViewport().setMaxX(arrayOfDataTemperatura.length-1);
            graph.getViewport().setDrawBorder(true);
            graph.getViewport().setScrollable(true); // enables horizontal scrolling
            graph.getViewport().setScrollableY(true); // enables vertical scrolling
            graph.getViewport().setScalable(true); // enables horizontal zooming and scrolling
            graph.getViewport().setScalableY(true); // enables vertical zooming and scrolling*/
            graph.getViewport().setXAxisBoundsManual(true);
            graph.getViewport().setMaxX(arrayOfDataHumedad.length);
            graph.getViewport().setMinX(0);
            graph.addSeries(serie);

            graph2.removeAllSeries();
            GridLabelRenderer glr2 = graph2.getGridLabelRenderer();
            GridLabelRenderer gridLabelRenderer2 = graph2.getGridLabelRenderer();
            gridLabelRenderer2.setGridStyle(GridLabelRenderer.GridStyle.HORIZONTAL);
            glr2.resetStyles();
            glr2.setHorizontalAxisTitle(xAxis);
            glr2.setVerticalAxisTitle("Temperatura (Cº)");
            LineGraphSeries serie2 = new LineGraphSeries(dataHumedad);
            serie2.setDrawDataPoints(true);
            serie2.setDrawAsPath(true);
            Paint paint2 = new Paint();
            paint2.setStyle(Paint.Style.STROKE);
            paint2.setStrokeWidth(2);
            serie2.setCustomPaint(paint);
            graph2.setTitle(titulo);
            graph2.getViewport().setMaxX(arrayOfDataHumedad.length);
            graph2.getViewport().setDrawBorder(true);
            graph2.getViewport().setScrollable(true); // enables horizontal scrolling
            graph2.getViewport().setScrollableY(true); // enables vertical scrolling
            graph2.getViewport().setScalable(true); // enables horizontal zooming and scrolling
            graph2.getViewport().setScalableY(true); // enables vertical zooming and scrolling*/
            graph2.getViewport().setXAxisBoundsManual(true);
            graph2.getViewport().setMaxX(arrayOfDataHumedad.length);
            graph2.getViewport().setMinX(0);
            graph2.addSeries(serie2);
        }
    }
}
