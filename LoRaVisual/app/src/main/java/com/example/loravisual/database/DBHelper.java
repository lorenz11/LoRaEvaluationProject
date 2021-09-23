package com.example.loravisual.database;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

/**
 * Helper class to create and access tables in the SQLite database.
 */
public class DBHelper extends SQLiteOpenHelper {
    String createString;

    /**
     * we use this constructor in @DatabaseHandler.
     * @param context
     * @param dbName
     * @param factory
     * @param version
     * @param createString
     */
    public DBHelper(Context context, String dbName, SQLiteDatabase.CursorFactory factory, int version, String createString) {
        super(context, dbName, factory, version);
        this.createString = createString;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(createString);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        onCreate(db);
    }
}
