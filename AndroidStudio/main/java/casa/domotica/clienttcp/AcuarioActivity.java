package casa.domotica.clienttcp;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class AcuarioActivity extends AppCompatActivity implements View.OnClickListener {
    Button buttonLed,buttonTiraLed;
    TextView responseTextView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_acuario);
        buttonLed = findViewById(R.id.buttonLed);
        buttonTiraLed = findViewById(R.id.buttonTiraLed);
        buttonLed.setOnClickListener(this);
        buttonTiraLed.setOnClickListener(this);
        ThreadAcuario threadAcuario = new ThreadAcuario(this,buttonTiraLed,buttonLed,"EstadoAcuario");
        threadAcuario.start();
    }

    @Override
    public void onClick(View v) {
        if(v.getId()== R.id.buttonLed){ //led
            ThreadAcuario threadAcuario = new ThreadAcuario(this,buttonTiraLed,buttonLed,"LEDS");
            threadAcuario.start();
        }
        else if(v.getId()== R.id.buttonTiraLed) {//tira
            ThreadAcuario threadAcuario = new ThreadAcuario(this,buttonTiraLed,buttonLed,"TIRA");
            threadAcuario.start();
        }
    }
}
