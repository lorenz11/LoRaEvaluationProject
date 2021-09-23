package com.example.loravisual.lvutilities;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import androidx.annotation.NonNull;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.firebase.storage.FirebaseStorage;
import com.google.firebase.storage.StorageReference;
import com.google.firebase.storage.UploadTask;

import java.io.File;

/**
 * This class uses the Android API for Firebase to upload experiment information and results.
 */
public class FirebaseHandler {
    private StorageReference storageReference;
    private Context context;

    public FirebaseHandler(Context context) {
        this.context = context;
    }

    /**
     * uploads one file. (called twice in @Client to upload both files: experiment_info.txt and experiment_data.csv)
     * @param path
     * @param child
     * @param filetype
     */
    public void uploadData(String path, String child, String filetype) {
        storageReference = FirebaseStorage.getInstance().getReference();

        Uri file = Uri.fromFile(new File(path));
        StorageReference dataRef = storageReference.child(child);

        dataRef.putFile(file)
                .addOnSuccessListener(new OnSuccessListener<UploadTask.TaskSnapshot>() {
                    @Override
                    public void onSuccess(UploadTask.TaskSnapshot taskSnapshot) {
                        Toast.makeText(context, filetype + " upload successful", Toast.LENGTH_SHORT).show();
                    }
                })
                .addOnFailureListener(new OnFailureListener() {
                    @Override
                    public void onFailure(@NonNull Exception e) {
                        Toast.makeText(context, filetype + " upload successful", Toast.LENGTH_SHORT).show();
                    }
                });
    }
}
