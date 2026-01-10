/*
 * Copyright (C) 2013 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.example.games.basegameutils;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;

import java.io.*;
import java.util.concurrent.Semaphore;
import java.util.Random;

import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.common.Scopes;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.GamesStatusCodes;
import com.google.android.gms.games.snapshot.Snapshot;
import com.google.android.gms.games.snapshot.SnapshotMetadata;
import com.google.android.gms.games.snapshot.SnapshotMetadataBuffer;
import com.google.android.gms.games.snapshot.SnapshotMetadataChange;
import com.google.android.gms.games.snapshot.Snapshots;

public class CloudSave implements GameHelper.GameHelperListener {

	// The game helper object. This class is mainly a wrapper around this object.
	protected GameHelper mHelper;
	public int REQUEST_CLOUD_SAVE_ID = 38;

	MainActivity parent;

	Semaphore semaphore;

	// ===== Public API =====

	public CloudSave(MainActivity p)
	{
		Log.i("SDL", "CloudSave: initializing");
		parent = p;
		semaphore = new Semaphore(0);
		mHelper = new GameHelper(parent, GameHelper.CLIENT_SNAPSHOT);
		mHelper.setMaxAutoSignInAttempts(0);
		mHelper.setup(this);
	}

	public void onStart()
	{
		mHelper.onStart(parent);
	}

	public void onStop()
	{
		mHelper.onStop();
	}

	SnapshotMetadata crapshotMetadata = null;
	boolean createNewSave = false;
	public void onActivityResult(int request, int response, Intent intent)
	{
		if (request != REQUEST_CLOUD_SAVE_ID)
			return;
		Log.d("SDL", "CloudSave: onActivityResult() response " + response + " intent " + (intent != null));
		try
		{
			if (intent != null && intent.hasExtra(Snapshots.EXTRA_SNAPSHOT_NEW))
			{
				createNewSave = true;
			}
			if (intent != null && intent.hasExtra(Snapshots.EXTRA_SNAPSHOT_METADATA))
			{
				crapshotMetadata = (SnapshotMetadata)
					intent.getParcelableExtra(Snapshots.EXTRA_SNAPSHOT_METADATA);
			}
		}
		catch (Exception e)
		{
			Log.w("SDL", "CloudSave: onActivityResult(): error: " + e.toString());
		}
		//mHelper.onActivityResult(request, response, data);
		semaphore.release();
	}

	public synchronized boolean save(String filename, String saveId, String dialogTitle, String description, String imageFile, long playedTimeMs)
	{
		Log.d("SDL", "CloudSave: save: file " + filename + " saveId " + saveId + " dialogTitle " + dialogTitle + " desc " + description + " imageFile " + imageFile + " playedTime " + playedTimeMs);

		if( !signIn() )
			return false;

		if( !filename.startsWith("/") )
			filename = Globals.DataDir + "/" + filename;
		if( imageFile.length() > 0 && !imageFile.startsWith("/") )
			imageFile = Globals.DataDir + "/" + imageFile;

		try
		{
			if( saveId == null || saveId.length() == 0 )
			{
				// Show dialog to the user
				// Specifying Snapshots.DISPLAY_LIMIT_NONE will cause the dialog to hide "New game" button for some reason
				final Intent snapshotIntent = Games.Snapshots.getSelectSnapshotIntent(getApiClient(), dialogTitle, true, true, 1000);
				semaphore.drainPermits();
				crapshotMetadata = null;
				createNewSave = false;
				parent.runOnUiThread(new Runnable()
				{
					public void run()
					{
						parent.startActivityForResult(snapshotIntent, REQUEST_CLOUD_SAVE_ID);
					}
				});
				semaphore.acquireUninterruptibly();
				Log.d("SDL", "CloudSave: save: user selected: " + (crapshotMetadata == null ? "null" : crapshotMetadata.getUniqueName()) + " new " + createNewSave);
				if( createNewSave )
					saveId = "" + System.currentTimeMillis() + "-" + new Random().nextInt(100000);
				else if( crapshotMetadata != null )
					saveId = crapshotMetadata.getUniqueName();
				else
					return false;
			}
			saveId = saveId.replaceAll("[^a-zA-Z0-9\\-._~]", "-");
			Log.d("SDL", "CloudSave: urlescaping saveId: " + saveId);
			Snapshots.OpenSnapshotResult result = Games.Snapshots.open(getApiClient(), saveId, true).await();
			Snapshot crapshot = processSnapshotOpenResult(result, 0);
			if( crapshot == null )
				return false;

			crapshot.getSnapshotContents().writeBytes(readFile(filename));

			Bitmap bmp = BitmapFactory.decodeFile(imageFile);
			while( bmp != null && bmp.getByteCount() > Games.Snapshots.getMaxCoverImageSize(getApiClient()) )
				bmp = Bitmap.createScaledBitmap(bmp, bmp.getWidth() * 3 / 4, bmp.getHeight() * 3 / 4, true);

			SnapshotMetadataChange.Builder metadataChange = new SnapshotMetadataChange.Builder()
				.setDescription(description)
				.setPlayedTimeMillis(playedTimeMs);
			if( bmp != null )
				metadataChange.setCoverImage(bmp);

			Games.Snapshots.commitAndClose(getApiClient(), crapshot, metadataChange.build())
				.setResultCallback(new ResultCallback<Snapshots.CommitSnapshotResult>()
				{
					public void onResult(Snapshots.CommitSnapshotResult r)
					{
						Log.i("SDL", "CloudSave: save final net sync result: " + r.getStatus().toString());
					}
				});

			Log.i("SDL", "CloudSave: save succeeded");
			return true;
		}
		catch(Exception e)
		{
			Log.i("SDL", "CloudSave: save failed: " + e.toString());
		}
		return false;
	}

	public synchronized boolean load(String filename, String saveId, String dialogTitle)
	{
		Log.d("SDL", "CloudSave: load: file " + filename + " saveId " + saveId + " dialogTitle " + dialogTitle);

		if( !signIn() )
			return false;

		if( !filename.startsWith("/") )
			filename = Globals.DataDir + "/" + filename;

		try
		{
			if( saveId == null || saveId.length() == 0 )
			{
				// Show dialog to the user
				final Intent snapshotIntent = Games.Snapshots.getSelectSnapshotIntent(getApiClient(), dialogTitle, false, true, 1000);
				semaphore.drainPermits();
				crapshotMetadata = null;
				createNewSave = false;
				parent.runOnUiThread(new Runnable()
				{
					public void run()
					{
						parent.startActivityForResult(snapshotIntent, REQUEST_CLOUD_SAVE_ID);
					}
				});
				semaphore.acquireUninterruptibly();
				Log.d("SDL", "CloudSave: load: user selected: " + (crapshotMetadata == null ? "null" : crapshotMetadata.getUniqueName()) + " new " + createNewSave);
				if( crapshotMetadata == null )
					return false;
				saveId = crapshotMetadata.getUniqueName();
			}

			Snapshots.OpenSnapshotResult result = Games.Snapshots.open(getApiClient(), saveId, false).await();
			if (result.getStatus().getStatusCode() != GamesStatusCodes.STATUS_OK)
			{
				Log.i("SDL", "CloudSave: load: failed to load game " + saveId + ": " + result.getStatus());
				return false;
			}

			boolean written = writeFile(filename, result.getSnapshot().getSnapshotContents().readFully());
			Log.i("SDL", "CloudSave: load: status: " + written);
			return written;
		}
		catch(Exception e)
		{
			Log.i("SDL", "CloudSave: load failed: " + e.toString());
		}
		return false;
	}

	public boolean isSignedIn() {
		return mHelper.isSignedIn();
	}

	// ===== Private API =====

	boolean signInSucceeded = false;
	boolean signInFailed = false;
	public boolean signIn()
	{
		//Log.i("SDL", "CloudSave: signIn()");
		if( !isSignedIn() )
		{
			signInSucceeded = false;
			signInFailed = false;
			Log.i("SDL", "CloudSave: beginUserInitiatedSignIn()");
			semaphore.drainPermits();
			beginUserInitiatedSignIn();
			semaphore.acquireUninterruptibly();
			return signInSucceeded;
		}
		return true;
	}


	public void onSignInSucceeded() {
		Log.i("SDL", "CloudSave: onSignInSucceeded()");
		signInSucceeded = true;
		semaphore.release();
	}

	public void onSignInFailed() {
		Log.i("SDL", "CloudSave: onSignInFailed()");
		signInFailed = true;
		semaphore.release();
	}

	public Snapshot processSnapshotOpenResult(Snapshots.OpenSnapshotResult result, int retryCount)
	{
		Snapshot mResolvedSnapshot = null;
		retryCount++;
		int status = result.getStatus().getStatusCode();

		Log.i("SDL", "CloudSave: processSnapshotOpenResult status: " + result.getStatus());

		if (status == GamesStatusCodes.STATUS_OK) {
			return result.getSnapshot();
		} else if (status == GamesStatusCodes.STATUS_SNAPSHOT_CONTENTS_UNAVAILABLE) {
			return result.getSnapshot();
		} else if (status == GamesStatusCodes.STATUS_SNAPSHOT_CONFLICT) {
			Snapshot snapshot = result.getSnapshot();
			Snapshot conflictSnapshot = result.getConflictingSnapshot();

			// Resolve between conflicts by selecting the newest of the conflicting snapshots.
			mResolvedSnapshot = snapshot;

			if (snapshot.getMetadata().getPlayedTime() == conflictSnapshot.getMetadata().getPlayedTime()) {
				if (snapshot.getMetadata().getLastModifiedTimestamp() < conflictSnapshot.getMetadata().getLastModifiedTimestamp()) {
					mResolvedSnapshot = conflictSnapshot;
				}
			} else if (snapshot.getMetadata().getPlayedTime() < conflictSnapshot.getMetadata().getPlayedTime()) {
				mResolvedSnapshot = conflictSnapshot;
			}

			Snapshots.OpenSnapshotResult resolveResult = Games.Snapshots.resolveConflict(
					getApiClient(), result.getConflictId(), mResolvedSnapshot)
					.await();

			if (retryCount < 3) {
				return processSnapshotOpenResult(resolveResult, retryCount);
			} else {
				Log.i("SDL", "CloudSave: could not resolve snapshot conflict");
			}
		}
		Log.i("SDL", "CloudSave: could not get savegame snapshot");
		return null;
	}

	static public byte[] readFile(String filename)
	{
		int len = (int)(new File(filename).length());
		if( len == 0 )
			return new byte[0];
		try
		{
			byte buf[] = new byte[len];
			if( new FileInputStream(filename).read(buf, 0, len) != len )
				return new byte[0];
			return buf;
		}
		catch( Exception e )
		{
			Log.i("SDL", "CloudSave: readFile() error: " + e.toString());
		}
		return new byte[0];
	}

	static public boolean writeFile(String filename, byte[] data)
	{
		try
		{
			BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(filename));
			out.write(data, 0, data.length);
			out.close();
			return true;
		}
		catch( Exception e )
		{
			Log.i("SDL", "CloudSave: writeFile() error: " + e.toString() + " file " + filename);
		}
		return false;
	}

	public GameHelper getGameHelper() {
		return mHelper;
	}

	public GoogleApiClient getApiClient() {
		return mHelper.getApiClient();
	}

	public void beginUserInitiatedSignIn() {
		mHelper.beginUserInitiatedSignIn();
	}

	public void signOut() {
		mHelper.signOut();
	}

	public void showAlert(String message) {
		mHelper.makeSimpleDialog(message).show();
	}

	public void showAlert(String title, String message) {
		mHelper.makeSimpleDialog(title, message).show();
	}

	public void enableDebugLog(boolean enabled) {
		mHelper.enableDebugLog(enabled);
	}

	public void reconnectClient() {
		mHelper.reconnectClient();
	}

	public boolean hasSignInError() {
		return mHelper.hasSignInError();
	}

	public GameHelper.SignInFailureReason getSignInError() {
		return mHelper.getSignInError();
	}
}
