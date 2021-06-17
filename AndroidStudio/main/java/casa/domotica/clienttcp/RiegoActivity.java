package casa.domotica.clienttcp;

import androidx.appcompat.app.AppCompatActivity;


import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class RiegoActivity extends AppCompatActivity implements View.OnClickListener {
    Button iniciarPrograma, iniciarZ1, iniciarZ2, apagarTodo;
    public TextView responseTextView;
    Boolean iniciarProgramaRiego = true;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_riego);
        iniciarPrograma = findViewById(R.id.button4);
        iniciarZ1 = findViewById(R.id.button6);
        iniciarZ2 = findViewById(R.id.button5);
        apagarTodo = findViewById(R.id.button8);
        iniciarPrograma.setOnClickListener(this);
        iniciarZ1.setOnClickListener(this);
        iniciarZ2.setOnClickListener(this);
        apagarTodo.setOnClickListener(this);
        ThreadRiego threadRiego = new ThreadRiego(this, iniciarZ1, iniciarZ2, iniciarPrograma, "EstadoRiego");
        threadRiego.start();

    }

    @Override
    public void onClick(View v) {
        int vez = 0;
        if (v.getId() == R.id.button4) { //iniciarPrograma
            ThreadRiego threadRiego = new ThreadRiego(this, iniciarZ1, iniciarZ2, iniciarPrograma, "IniciarPrograma");
            threadRiego.start();
            if(iniciarProgramaRiego) {
                BackgroundTaskEstadoRiego backgroundTaskEstadoRiego = new BackgroundTaskEstadoRiego(this);
                backgroundTaskEstadoRiego.execute("EstadoRiego");
            }
        } else if (v.getId() == R.id.button6) {//iniciar Z1
            ThreadRiego threadRiego = new ThreadRiego(this, iniciarZ1, iniciarZ2, iniciarPrograma, "IniciarZ1");
            threadRiego.start();
        } else if (v.getId() == R.id.button5) {//iniciar Z2
            ThreadRiego threadRiego = new ThreadRiego(this, iniciarZ1, iniciarZ2, iniciarPrograma, "IniciarZ2");
            threadRiego.start();
        } else if (v.getId() == R.id.button8) {//apagarTodo
            ThreadRiego threadRiego = new ThreadRiego(this, iniciarZ1, iniciarZ2, iniciarPrograma, "ApagarTodo");
            threadRiego.start();
        }
    }
}

