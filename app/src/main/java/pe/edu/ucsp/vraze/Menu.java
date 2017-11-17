package pe.edu.ucsp.vraze;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.GamesActivityResultCodes;
import com.google.android.gms.games.GamesStatusCodes;
import com.google.android.gms.games.multiplayer.Invitation;
import com.google.android.gms.games.multiplayer.Multiplayer;
import com.google.android.gms.games.multiplayer.OnInvitationReceivedListener;
import com.google.android.gms.games.multiplayer.Participant;
import com.google.android.gms.games.multiplayer.realtime.Room;
import com.google.android.gms.games.multiplayer.realtime.RoomConfig;
import com.google.android.gms.games.multiplayer.realtime.RoomStatusUpdateListener;
import com.google.android.gms.games.multiplayer.realtime.RoomUpdateListener;
import com.google.example.games.basegameutils.BaseGameUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Created by zafiron on 27/10/17.
 */


public class Menu extends Activity implements GoogleApiClient.ConnectionCallbacks, GoogleApiClient.OnConnectionFailedListener,
    View.OnClickListener,
    RoomStatusUpdateListener, RoomUpdateListener {


  public static int mScore = 0; // user's current score

  // Are we playing in multiplayer mode?
  static boolean mMultiplayer = false;

  // My participant ID in the currently active game
  public static String mMyId = null;


  // Message buffer for sending messages
  static byte[] mMsgBuf = new byte[4];


  // The participants in the currently active game
  public static ArrayList<Participant> mParticipants = null;
  // Has the user clicked the sign-in button?
  private boolean mSignInClicked = false;
  // Are we currently resolving a connection failure?
  private boolean mResolvingConnectionFailure = false;
  // Set to true to automatically start the sign in flow when the Activity starts.
  // Set to false to require the user to click the button in order to sign in.
  private boolean mAutoStartSignInFlow = true;
  // Room ID where the currently active game is taking place; null if we're
  // not playing.
  static String mRoomId = null;


  // Request codes for the UIs that we show with startActivityForResult:
  final static int RC_SELECT_PLAYERS = 10000;
  final static int RC_WAITING_ROOM = 10002;
  // Request code used to invoke sign in user interactions.
  private static final int RC_SIGN_IN = 9001;


  Button btn_quick_game;
  Button btn_single_player;
  Button btn_google_sign_out;
  com.google.android.gms.common.SignInButton btn_google_sign_in;
  // Client used to interact with Google APIs.
  public static GoogleApiClient mGoogleApiClient;
  final static String TAG = "vraZE";


  public static final String EXTRA_MESSAGE = "com.example.myfirstapp.MESSAGE";


  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main_menu);

    btn_google_sign_in = (com.google.android.gms.common.SignInButton) findViewById(pe.edu.ucsp.vraze.R.id.button_sign_in);
    btn_quick_game = (Button) findViewById(R.id.button_quick_game);
    btn_single_player = (Button) findViewById(R.id.button_single_player);
    btn_google_sign_out = (Button) findViewById(R.id.button_sign_out);
    // Create the Google Api Client with access to Games
    mGoogleApiClient = new GoogleApiClient.Builder(this)
        .addConnectionCallbacks(this)
        .addOnConnectionFailedListener(this)
        .addApi(Games.API).addScope(Games.SCOPE_GAMES)
        .build();



    btn_quick_game.setOnClickListener(new View.OnClickListener() {
      public void onClick(View v) {
        Log.d(TAG, "click btn_quick_game");
        startQuickGame();
      }
    });
    btn_single_player.setOnClickListener(new View.OnClickListener() {
      public void onClick(View v) {

        Log.d(TAG, "click btn_single_player");

        Intent Vraze = new Intent(getApplicationContext(), MainActivity.class);
        Vraze.putExtra("mParticipants", mParticipants);
        startActivity(Vraze);
      }
    });
    btn_google_sign_in.setOnClickListener(this);
    btn_google_sign_out.setOnClickListener(this);

  }

  @Override
  public void onClick(View v) {

    switch (v.getId()) {

      case R.id.button_sign_in:
        // user wants to sign in
        // Check to see the developer who's running this sample code read the instructions :-)
        // NOTE: this check is here only because this is a sample! Don't include this
        // check in your actual production app.
        if (!BaseGameUtils.verifySampleSetup(this, R.string.app_id)) {
          Log.w(TAG, "*** Warning: setup problems detected. Sign in may not work!");
        }

        // start the sign-in flow
        Log.d(TAG, "Sign-in button clicked");
        mSignInClicked = true;
        mGoogleApiClient.connect();
        break;
      case R.id.button_sign_out:
        // user wants to sign out
        // sign out.
        Log.d(TAG, "Sign-out button clicked");
        mSignInClicked = false;
        Games.signOut(mGoogleApiClient);
        mGoogleApiClient.disconnect();
        btn_google_sign_in.setVisibility(View.VISIBLE);
        btn_google_sign_out.setVisibility(View.INVISIBLE);
        btn_quick_game.setVisibility(View.INVISIBLE);
        break;
    }
  }

  @Override
  public void onActivityResult(int requestCode, int responseCode,
                               Intent intent) {
    super.onActivityResult(requestCode, responseCode, intent);

    switch (requestCode) {
      case RC_SELECT_PLAYERS:
        // we got the result from the "select players" UI -- ready to create the room
        handleSelectPlayersResult(responseCode, intent);
        break;
      case RC_WAITING_ROOM:
        // we got the result from the "waiting room" UI.
        if (responseCode == Activity.RESULT_OK) {
          // ready to start playing
          Log.d(TAG, "Starting game (waiting room returned OK).");
          startGame(true);
        } else if (responseCode == GamesActivityResultCodes.RESULT_LEFT_ROOM) {
          // player indicated that they want to leave the room
          leaveRoom();
        } else if (responseCode == Activity.RESULT_CANCELED) {
          // Dialog was cancelled (user pressed back key, for instance). In our game,
          // this means leaving the room too. In more elaborate games, this could mean
          // something else (like minimizing the waiting room UI).
          leaveRoom();
        }
        break;
      case RC_SIGN_IN:
        Log.d(TAG, "onActivityResult with requestCode == RC_SIGN_IN, responseCode="
            + responseCode + ", intent=" + intent);
        mSignInClicked = false;
        mResolvingConnectionFailure = false;
        if (responseCode == RESULT_OK) {
          mGoogleApiClient.connect();
          btn_google_sign_in.setVisibility(View.INVISIBLE);
          btn_google_sign_out.setVisibility(View.VISIBLE);
          btn_quick_game.setVisibility(View.VISIBLE);
        } else {
          BaseGameUtils.showActivityResultError(this, requestCode, responseCode, R.string.signin_other_error);
          btn_google_sign_in.setVisibility(View.VISIBLE);
          btn_google_sign_out.setVisibility(View.INVISIBLE);
          btn_quick_game.setVisibility(View.INVISIBLE);
        }
        break;
    }
    super.onActivityResult(requestCode, responseCode, intent);
  }

  // Start the gameplay phase of the game.
  void startGame(boolean multiplayer) {
    mMultiplayer = multiplayer;


    Intent intent = new Intent(this, MainActivity.class);
    startActivity(intent);
  }

  // Handle the result of the "Select players UI" we launched when the user clicked the
  // "Invite friends" button. We react by creating a room with those players.
  private void handleSelectPlayersResult(int response, Intent data) {
    if (response != Activity.RESULT_OK) {
      Log.w(TAG, "*** select players UI cancelled, " + response);
      return;
    }

    Log.d(TAG, "Select players UI succeeded.");

    // get the invitee list
    final ArrayList<String> invitees = data.getStringArrayListExtra(Games.EXTRA_PLAYER_IDS);
    Log.d(TAG, "Invitee count: " + invitees.size());

    // get the automatch criteria
    Bundle autoMatchCriteria = null;
    int minAutoMatchPlayers = data.getIntExtra(Multiplayer.EXTRA_MIN_AUTOMATCH_PLAYERS, 0);
    int maxAutoMatchPlayers = data.getIntExtra(Multiplayer.EXTRA_MAX_AUTOMATCH_PLAYERS, 0);
    if (minAutoMatchPlayers > 0 || maxAutoMatchPlayers > 0) {
      autoMatchCriteria = RoomConfig.createAutoMatchCriteria(
          minAutoMatchPlayers, maxAutoMatchPlayers, 0);
      Log.d(TAG, "Automatch criteria: " + autoMatchCriteria);
    }

    // create the room
    Log.d(TAG, "Creating room...");
    RoomConfig.Builder rtmConfigBuilder = RoomConfig.builder(this);
    rtmConfigBuilder.addPlayersToInvite(invitees);
    rtmConfigBuilder.setMessageReceivedListener(MainActivity.mMessageReceivedListener);
    rtmConfigBuilder.setRoomStatusUpdateListener(this);
    if (autoMatchCriteria != null) {
      rtmConfigBuilder.setAutoMatchCriteria(autoMatchCriteria);
    }
    //switchToScreen(pe.ucsp.example.games.bc.R.id.screen_wait);
    keepScreenOn();
    Games.RealTimeMultiplayer.create(mGoogleApiClient, rtmConfigBuilder.build());
    Log.d(TAG, "Room created, waiting for it to be ready...");
  }

  // Leave the room.
  void leaveRoom() {
    Log.d(TAG, "Leaving room.");
    //mSecondsLeft = 0;
    stopKeepingScreenOn();
    if (mRoomId != null) {
      Games.RealTimeMultiplayer.leave(mGoogleApiClient, this, mRoomId);
      mRoomId = null;
    }
  }

  void startQuickGame() {
    // quick-start a game with 1 randomly selected opponent
    final int MIN_OPPONENTS = 1, MAX_OPPONENTS = 1;
    Bundle autoMatchCriteria = RoomConfig.createAutoMatchCriteria(MIN_OPPONENTS,
        MAX_OPPONENTS, 0);
    RoomConfig.Builder rtmConfigBuilder = RoomConfig.builder(this);
    rtmConfigBuilder.setMessageReceivedListener(MainActivity.mMessageReceivedListener);
    rtmConfigBuilder.setRoomStatusUpdateListener(this);
    rtmConfigBuilder.setAutoMatchCriteria(autoMatchCriteria);
    keepScreenOn();

    Games.RealTimeMultiplayer.create(mGoogleApiClient, rtmConfigBuilder.build());
  }

  // Sets the flag to keep this screen on. It's repemended to do that during
  // the
  // handshake when setting up a game, because if the screen turns off, the
  // game will be
  // cancelled.
  void keepScreenOn() {
    getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
  }


  // Show error message about game being cancelled and return to main screen.
  void showGameError() {
    BaseGameUtils.makeSimpleDialog(this, getString(R.string.game_problem));
  }

  // Clears the flag that keeps the screen on.
  void stopKeepingScreenOn() {
    getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
  }

  // Show the waiting room UI to track the progress of other players as they enter the
  // room and get connected.
  void showWaitingRoom(Room room) {
    // minimum number of players required for our game
    // For simplicity, we require everyone to join the game before we start it
    // (this is signaled by Integer.MAX_VALUE).
    final int MIN_PLAYERS = Integer.MAX_VALUE;
    Intent i = Games.RealTimeMultiplayer.getWaitingRoomIntent(mGoogleApiClient, room, MIN_PLAYERS);

    // show waiting room UI
    startActivityForResult(i, RC_WAITING_ROOM);
  }

  void updateRoom(Room room) {
    if (room != null) {
      mParticipants = room.getParticipants();
    }

  }



  // Activity just got to the foreground. We switch to the wait screen because we will now
  // go through the sign-in flow (remember that, yes, every time the Activity pees back to the
  // foreground we go through the sign-in flow -- but if the user is already authenticated,
  // this flow simply succeeds and is imperceptible).
  @Override
  public void onStart() {
    if (mGoogleApiClient == null) {
      //switchToScreen(pe.edu.ucsp.vraze.R.id.screen_sign_in);
    } else if (!mGoogleApiClient.isConnected()) {
      Log.d(TAG, "Connecting client.");
      //switchToScreen(pe.edu.ucsp.vraze.R.id.screen_wait);
      //mGoogleApiClient.connect();
    } else {
      Log.w(TAG,
          "GameHelper: client was already connected on onStart()");
    }
    super.onStart();
  }


  @Override
  public void onConnected(@Nullable Bundle connectionHint) {
    Log.d(TAG, "Conectado con exito!!! :)");
    Log.d(TAG, "onConnected() called. Sign in successful!");

    Log.d(TAG, "Sign-in succeeded.");


  }

  @Override
  public void onConnectionSuspended(int i) {
    Log.d(TAG, "onConnectionSuspended() called. Trying to reconnect.");
    mGoogleApiClient.connect();
  }

  @Override
  public void onConnectionFailed(@NonNull ConnectionResult connectionResult) {
    Log.d(TAG, "onConnectionFailed() called, result: " + connectionResult);

    if (mResolvingConnectionFailure) {
      Log.d(TAG, "onConnectionFailed() ignoring connection failure; already resolving.");
      return;
    }

    if (mSignInClicked || mAutoStartSignInFlow) {
      mAutoStartSignInFlow = false;
      mSignInClicked = false;
      mResolvingConnectionFailure = BaseGameUtils.resolveConnectionFailure(this, mGoogleApiClient,
          connectionResult, RC_SIGN_IN, getString(R.string.signin_other_error));
    }
  }


  /*
  * todo lo referente a salas de juegos
  * */
  @Override
  public void onRoomCreated(int statusCode, Room room) {
    Log.d(TAG, "onRoomCreated(" + statusCode + ", " + room + ")");
    if (statusCode != GamesStatusCodes.STATUS_OK) {
      Log.e(TAG, "*** Error: onRoomCreated, status " + statusCode);
      showGameError();
      return;
    }

    // save room ID so we can leave cleanly before the game starts.
    mRoomId = room.getRoomId();

    // show the waiting room UI
    showWaitingRoom(room);
  }

  @Override
  public void onJoinedRoom(int statusCode, Room room) {
    Log.d(TAG, "onJoinedRoom(" + statusCode + ", " + room + ")");
    if (statusCode != GamesStatusCodes.STATUS_OK) {
      Log.e(TAG, "*** Error: onRoomConnected, status " + statusCode);
      showGameError();
      return;
    }

    // show the waiting room UI
    showWaitingRoom(room);
  }

  @Override
  public void onLeftRoom(int statusCode, String s) {
    Log.d(TAG, "onLeftRoom, code " + statusCode);

  }

  @Override
  public void onRoomConnected(int statusCode, Room room) {
    Log.d(TAG, "onRoomConnected(" + statusCode + ", " + room + ")");
    if (statusCode != GamesStatusCodes.STATUS_OK) {
      Log.e(TAG, "*** Error: onRoomConnected, status " + statusCode);
      showGameError();
      return;
    }
    updateRoom(room);
  }


  @Override
  public void onRoomConnecting(Room room) {
    updateRoom(room);
  }

  @Override
  public void onRoomAutoMatching(Room room) {
    updateRoom(room);

  }
    /*fin de todo lo referente a salas de juego*/


  @Override
  public void onPeerInvitedToRoom(Room room, List<String> list) {
    updateRoom(room);

  }

  // We treat most of the room update callbacks in the same way: we update our list of
  // participants and update the display. In a real game we would also have to check if that
  // change requires some action like removing the corresponding player avatar from the screen,
  // etc.
  @Override
  public void onPeerDeclined(Room room, List<String> list) {
    updateRoom(room);

  }

  @Override
  public void onPeerJoined(Room room, List<String> list) {
    updateRoom(room);

  }

  @Override
  public void onPeerLeft(Room room, List<String> list) {
    updateRoom(room);

  }

  // Called when we are connected to the room. We're not ready to play yet! (maybe not everybody
  // is connected yet).
  @Override
  public void onConnectedToRoom(Room room) {
    Log.d(TAG, "onConnectedToRoom.");

    //get participants and my ID:
    mParticipants = room.getParticipants();
    mMyId = room.getParticipantId(Games.Players.getCurrentPlayerId(mGoogleApiClient));

    // save room ID if its not initialized in onRoomCreated() so we can leave cleanly before the game starts.
    if (mRoomId == null)
      mRoomId = room.getRoomId();

    // print out the list of participants (for debug purposes)
    Log.d(TAG, "Room ID: " + mRoomId);
    Log.d(TAG, "My ID " + mMyId);
    Log.d(TAG, "<< CONNECTED TO ROOM>>");
  }

  // Called when we get disconnected from the room. We return to the main screen.
  @Override
  public void onDisconnectedFromRoom(Room room) {
    mRoomId = null;
    showGameError();
  }

  @Override
  public void onPeersConnected(Room room, List<String> list) {
    updateRoom(room);
  }

  @Override
  public void onPeersDisconnected(Room room, List<String> list) {
    updateRoom(room);
  }

  @Override
  public void onP2PConnected(String s) {
//nada
  }

  @Override
  public void onP2PDisconnected(String s) {
//nada
  }

  // Called when we get an invitation to play a game. We react by showing that to the user.



  // Broadcast my score to everybody else.
  public static void broadcastScore(float x, float y , float dir0, float dir1,boolean is_finish) {
    if (!mMultiplayer)
      return; // playing single-player mode


    mMsgBuf[0] = (byte) x;
    mMsgBuf[1] = (byte) 'B';
    Log.d(TAG, "envio: " + mMsgBuf);
    // Send to every other participant.
    for (Participant p : mParticipants) {
      if (p.getParticipantId().equals(mMyId))
        continue;
      if (p.getStatus() != Participant.STATUS_JOINED)
        continue;
      if (is_finish) {
        // final score notification must be sent via reliable message
        Games.RealTimeMultiplayer.sendReliableMessage(mGoogleApiClient, null, mMsgBuf,
            mRoomId, p.getParticipantId());
      } else {
        // it's an interim score notification, so we can use unreliable
        Games.RealTimeMultiplayer.sendUnreliableMessage(mGoogleApiClient, mMsgBuf, mRoomId,
            p.getParticipantId());
      }
    }
  }
}
