package casa.domotica.clienttcp;

import androidx.appcompat.app.AppCompatActivity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends AppCompatActivity implements View.OnClickListener{
    EditText ip, message;
    Button bAcuario, bClima, bRiego, buttonWebPage;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        bAcuario = findViewById(R.id.buttonAcuario);
        bClima = findViewById(R.id.buttonClima);
        bRiego = findViewById(R.id.buttonRiego);
        buttonWebPage = findViewById(R.id.buttonWebPage);
        buttonWebPage.setOnClickListener(this);
        bAcuario.setOnClickListener(this);
        bClima.setOnClickListener(this);
        bRiego.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.buttonAcuario) {
            Intent acuario = new Intent(this, AcuarioActivity.class);
            startActivity(acuario);
        } else if (v.getId() == R.id.buttonRiego) {
            Intent riego = new Intent(this, RiegoActivity.class);
            startActivity(riego);
        } else if (v.getId() == R.id.buttonWebPage) {
            Intent webPage = new Intent (Intent.ACTION_VIEW, Uri.parse("http://domosapienstfg.ddns.net/"));
            startActivity(webPage);
        } else if (v.getId() == R.id.buttonClima) {
            Intent clima = new Intent (this, Clima.class);
            startActivity(clima);
        }
    }
}

