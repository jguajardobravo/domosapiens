package casa.domotica.clienttcp;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GridLabelRenderer;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class Clima extends AppCompatActivity implements View.OnClickListener{
    String escala = "";
    GraphView graph,graph2;
    Button buttonAno,buttonMes,buttonDia,buttonHora,buttonAnterior,buttonSiguiente;
    int numero = 0;
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_clima);
        graph = findViewById(R.id.graph);
        graph2 = findViewById(R.id.graph2);
        buttonAno = findViewById(R.id.button);
        buttonMes = findViewById(R.id.button3);
        buttonDia = findViewById(R.id.button7);
        buttonHora = findViewById(R.id.button9);
        buttonAno.setOnClickListener(this);
        buttonMes.setOnClickListener(this);
        buttonDia.setOnClickListener(this);
        buttonHora.setOnClickListener(this);
        ThreadClima threadClima = new ThreadClima(this,graph,graph2,"Temperatura");
        threadClima.start();
    }


    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.button){
            ThreadClima threadClima = new ThreadClima (this,graph,graph2,"TempAno");
            threadClima.start();
        }
        else if(v.getId() == R.id.button3){
            ThreadClima threadClima = new ThreadClima (this,graph,graph2,"TempMes");
            threadClima.start();
        }
        else if(v.getId() == R.id.button7){
            ThreadClima threadClima = new ThreadClima (this,graph,graph2,"TempDia");
            threadClima.start();
        }
        else if(v.getId() == R.id.button9){
            ThreadClima threadClima = new ThreadClima (this,graph,graph2,"Temperatura");
            threadClima.start();

        }
    }
}
