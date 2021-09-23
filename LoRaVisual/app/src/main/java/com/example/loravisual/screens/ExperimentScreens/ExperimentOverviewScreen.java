package com.example.loravisual.screens.ExperimentScreens;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.example.loravisual.R;
import com.example.loravisual.database.DatabaseHandler;

import java.util.ArrayList;

/**
 * this screen lists all experiments, both only yet created and finished.
 */
public class ExperimentOverviewScreen extends AppCompatActivity {
    private static final String TAG = "at ExperimentOverviewScreen";

    DatabaseHandler databaseHandler = new DatabaseHandler(this);

    Context context = this;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    /**
     * adds a "+" button to the menu in the top right of the screen, which leads to @CustomizeExperimentScreen
     */
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.add_experiment_menu, menu);
        return true;
    }

    @Override
    /**
     * defines behaviour for "+" button.
     */
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.new_experiment) {
            Intent intent = new Intent(this, CustomizeExperimentsScreen.class);
            startActivity(intent);
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    /**
     * we implement this part of the app life cycle (apart from onCreate), so that
     * if a new experiment was created in @CustomizeExperimentScreen and a user presses the back button,
     * the experiment can already appear in the overview.
     */
    protected void onStart() {
        super.onStart();

        setContentView(R.layout.activity_experiment_overview_screen);
        getSupportActionBar().setTitle("Experiment Overview");

        ListView listView = findViewById(R.id.listview);
        ArrayList<String> list = databaseHandler.getPrimaryColumn(true);

        ArrayAdapter adapter = new ArrayAdapter(this, R.layout.list_item_view, list);
        listView.setAdapter(adapter);

        ArrayList<String> absoluteIds = databaseHandler.getPrimaryColumn(false);
        listView.setOnItemClickListener((adapterView, view, i, l) -> {
            Intent intent;
            int id = Integer.parseInt(absoluteIds.get(i));



            if(id == 1 || id == 54) {     //if(id == 1 || id ==2)
                databaseHandler.setValueInExpInfo(DatabaseHandler.READY, "state", id);
            }



            if(databaseHandler.getSingleExperimentInfoValue(id, DatabaseHandler.STATE).equals(DatabaseHandler.READY)) {
                intent = new Intent(this, PrepareSenderScreen.class);
                intent.putExtra("absolutePos", id);
            } else {
                intent = new Intent(this, StartExperimentScreen.class);
                intent.putExtra("absolutePos", id);
            }
            startActivity(intent);
        });
        listView.setOnItemLongClickListener((adapterView, view, i, l) -> {
            new AlertDialog.Builder(this)
                    .setTitle("Delete Entry?")
                    .setMessage("Are you sure you want to delete experiment " + list.get(i) + "?")
                    .setCancelable(true)
                    .setPositiveButton("confirm", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int j) {
                            databaseHandler.deleteExperiment(Integer.parseInt(absoluteIds.get(i)));
                            Toast.makeText(context, "Deleted experiment " + list.get(i) + " and all associated data", Toast.LENGTH_SHORT).show();
                            finish();
                            startActivity(getIntent());
                        }
                    }).show();

            return true;
        });

    }
}