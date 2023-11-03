#include <Wire.h>
#include <MFRC522.h>
#include <typeinfo>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Arduino.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
using namespace std;

#define SS_PIN 12
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

const char *rootCACertificate = "";

WiFiClientSecure *client = new WiFiClientSecure;
unsigned long myTime;
unsigned long StartTime;
unsigned long BeginSong;
unsigned long DurationSong;
unsigned long added_time = 0;
String last_hex = "";
String loop_until_ID()
{
  while (1)
  {
    if (!mfrc522.PICC_IsNewCardPresent())
    {
      continue;
    }
    if (!mfrc522.PICC_ReadCardSerial())
    {
      continue;
    }
    unsigned long hex_num;
    hex_num = mfrc522.uid.uidByte[0] << 24;
    hex_num += mfrc522.uid.uidByte[1] << 16;
    hex_num += mfrc522.uid.uidByte[2] << 8;
    hex_num += mfrc522.uid.uidByte[3];
    mfrc522.PICC_HaltA();
    return String(hex_num);
  }
}
class Spotify
{
public:
  int pause_play = 1;
  bool stup = false;
  int index = 0;
  JSONVar every_album;   //"hex":"album uri(raw)"
  JSONVar current_album; // index : "number uri(raw)"
  String spotify_token = "";
  String spotify_id = "";
  String refresh_token = "";
  String client_id = "";
  bool shuffle_state = false;
  int volume_percent = 0;
  String device_id = "";

  void setup()
  {
    updateToken();
    while (!stup)
    {
      // WiFiClientSecure *client = new WiFiClientSecure;
      if (client)
      {
        client->setCACert(rootCACertificate);
        {
          HTTPClient http;
          if (http.begin(*client, "https://api.spotify.com/v1/me/player"))
          {
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Authorization", "Bearer " + spotify_token);
            int httpResponseCode = http.GET();

            if (httpResponseCode == 200)
            {
              stup = true;
            }
            else
            {
              Serial.println("Error with setup:");
              Serial.println(httpResponseCode);
              return;
            }
            String payload = http.getString();

            JSONVar data = JSON.parse(payload);

            shuffle_state = data["shuffle_state"];
            volume_percent = data["device"]["volume_percent"];
            device_id = data["device"]["id"];
            http.end();
          }
          else
          {
            Serial.printf("[HTTPS] Unable to connect\n");
          }
        }
        // delete client;
      }
      else
      {
        Serial.println("Unable to create client");
      }
      // delete client;
    }
  }

  void next()
  {
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/me/player/next"))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.POST("{}");
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
  }

  void previous()
  {
    Serial.println("Previous");
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/me/player/previous"))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.POST("{}");
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
  }

  bool updateToken()
  {
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://accounts.spotify.com/api/token"))
        {
          http.addHeader("Authorization", "Basic " + client_id);
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          String httpRequestData = "grant_type=refresh_token&refresh_token=" + refresh_token;
          http.POST(httpRequestData);
          String payload = http.getString();
          JSONVar data = JSON.parse(payload);
          spotify_token = data["access_token"];
          http.end();
          return true;
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
          return false;
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
      return false;
    }
  }

  void play()
  {
    String tmp = "play";
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/me/player/play"))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.PUT("{}");
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
      // delete client;
    }
    else
    {
      Serial.println("Unable to create client");
    }
  }

  void Pause()
  {
    String tmp = "https://api.spotify.com/v1/me/player/pause";
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, tmp))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.PUT("{}");
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
  }

  void setVolume(int volume)
  {
    volume_percent = volume;
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/me/player/volume?volume_percent=" + String(volume)))
        {
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.PUT("{}");
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
  }

  String encode(String &url)
  {
    String tmp = "";
    for (char const &ltr : url)
    {
      if (ltr == ' ' || ltr == '/' || ltr == '\\')
      {
        continue;
      }
      tmp += ltr;
    }
    return url;
  }

  JSONVar getDevices()
  {
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/me/player/devices"))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.GET();
          String payload = http.getString();
          JSONVar data = JSON.parse(payload);
          return data;
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
    JSONVar o;
    return o;
  }

  void changePlaybackDevice(String id)
  {
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/me/player"))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.PUT("{\"device_ids\":[\"" + id + "\"] , \"play\":true}");
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
  }

  String fix(String str)
  {
    str.replace("\"", "");
    return str;
  }

  JSONVar getAlbumTracks(String uri)
  { // uncut uri:)
    int totalTracks = 99;
    int currentNumber = 0;
    JSONVar albumTracks;
    while (currentNumber < totalTracks)
    {
      JSONVar temp = getAlbumTracks2(uri, currentNumber);
      for (int i = 0; i < temp["list"].length(); i++)
      {
        JSONVar x;
        String songName = fix(JSON.stringify(temp["list"][i]["songName"]));
        String uri = fix(JSON.stringify(temp["list"][i]["uri"]));
        String id = fix(JSON.stringify(temp["list"][i]["id"]));
        x["songName"] = songName;
        x["uri"] = uri;
        x["id"] = id;
        albumTracks[currentNumber + i] = x;
      }
      totalTracks = fix(JSON.stringify(temp["total"])).toInt();
      currentNumber += 5;
    }
    return albumTracks;
  }
  JSONVar getAlbumTracks2(String uri, int offset)
  {
    JSONVar returnList;
    uri = uri.substring(14);
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/albums/" + uri + "/tracks?offset=" + String(offset) + "&limit=5"))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.GET();
          String payload = http.getString();
          JSONVar data = JSON.parse(payload);
          for (int i = 0; i < data["items"].length(); i++)
          {
            JSONVar tmp;
            String songName = JSON.stringify(data["items"][i]["name"]);
            songName.replace("\"", "");
            tmp["songName"] = songName;
            String uri = JSON.stringify(data["items"][i]["uri"]);
            uri.replace("\"", "");
            tmp["uri"] = uri;
            String id = JSON.stringify(data["items"][i]["id"]);
            id.replace("\"", "");
            tmp["id"] = id;
            returnList["list"][i] = tmp;
          }
          returnList["total"] = JSON.stringify(data["total"]);
          return returnList;
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
    return returnList;
  }

  void addToQue(String uri)
  {
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/me/player/queue?uri=" + String(uri)))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.POST("{}");
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
  }
  void changeToRaspotify()
  {
    changePlaybackDevice("672af9fd7f27789dfab0afcc92758fb017b24477");
  }

  void ToQueFromIndex(int indexToPlay)
  {
    String uri = fix(JSON.stringify(current_album[indexToPlay]["uri"]));
    addToQue(uri);
    added_time = 0;
    DurationSong = 0;
  }
  void changeCurrentlyPlayingAlbum(String albumUri)
  {
    current_album = getAlbumTracks(albumUri); // uncut uri
    index = 0;
    ToQueFromIndex(0);
    next();
  }

  struct ret
  {
    int progress = 1;
    int duration = 10;
    int index = 0;
  };

  struct ret getTimeRemaining()
  {

    struct ret returnStruct;
    // WiFiClientSecure *client = new WiFiClientSecure;
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/me/player"))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.GET();
          String payload = http.getString();
          JSONVar data = JSON.parse(payload);
          http.end();

          int progress = fix(JSON.stringify(data["progress_ms"])).toInt();
          int duration = fix(JSON.stringify(data["item"]["duration_ms"])).toInt();
          int ind = fix(JSON.stringify(data["item"]["track_number"])).toInt();
          returnStruct.duration = duration;
          returnStruct.progress = progress;
          returnStruct.index = ind;
          return returnStruct;
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
      // delete client;
    }
    else
    {
      Serial.println("Unable to create client");
    }
    // delete client;
    return returnStruct;
  }

  void setupAlbums()
  {
    String hex;
    // get songs in cpp project playlist
    JSONVar playlistInfo = getPlaylist();

    String arrayHEX[playlistInfo.length()];

    for (int i = 0; i < playlistInfo.length(); i++)
    {
      // get card in hex, plz be string
      Serial.print("Scan Album: ");
      Serial.println(playlistInfo[i]["albumName"]);

      while (1)
      {
        String tempHEX = loop_until_ID();

        bool contains = false;
        for (int x = 0; x < arrayHEX->length(); x++)
        {
          if (tempHEX == arrayHEX[x])
          {
            contains = true;
            break;
          }
        }
        if (contains)
        {
          Serial.println("Scan a new one please");
          continue;
        }
        hex = tempHEX;
        break;
      }
      arrayHEX[i] = hex;
      Serial.print("Found: ");
      Serial.println(hex);
      playlistInfo[i]["hex"] = hex;
      String formatUri = JSON.stringify(playlistInfo[i]["albumUri"]);
      formatUri.replace("\"", "");
      playlistInfo[i]["formatUri"] = formatUri.substring(14);
    }
    every_album = playlistInfo;
  }

  JSONVar getPlaylist()
  {
    JSONVar returnList;
    // 0ixAQnWrlGFAusKA0e17RE = uri playlist
    if (client)
    {
      client->setCACert(rootCACertificate);
      {
        HTTPClient http;
        if (http.begin(*client, "https://api.spotify.com/v1/playlists/0ixAQnWrlGFAusKA0e17RE"))
        {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", "Bearer " + spotify_token);
          http.GET();
          String payload = http.getString();

          JSONVar tmp = JSON.parse(payload);
          for (int i = 0; i < tmp["tracks"]["items"].length(); i++)
          {
            String albumName = JSON.stringify(tmp["tracks"]["items"][i]["track"]["album"]["name"]);
            String albumUri = JSON.stringify(tmp["tracks"]["items"][i]["track"]["album"]["uri"]);
            albumName.replace("\"", "");
            albumUri.replace("\"", "");
            JSONVar o;
            o["albumName"] = albumName;
            o["albumUri"] = albumUri;
            returnList[i] = o;
          }
          http.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
    else
    {
      Serial.println("Unable to create client");
    }
    return returnList;
  }

  void handleAutomaticNext()
  {
    if (!pause_play)
    {
      return;
    }
    if (DurationSong == 0)
    {
      struct ret response = getTimeRemaining();
      if (response.index == index + 1)
      {
        DurationSong = response.duration;
        BeginSong = millis();
      }
    }
    if (DurationSong != 0 && DurationSong < ((millis() - BeginSong) + 7000) - added_time)
    {
      if (current_album.length() > index + 1)
      {
        index += 1;
        ToQueFromIndex(index);
      }
      else
      {
        Pause();
        index += 1;
      }
    }
  }

  void playNext()
  {
    index += 1;
    if (index >= current_album.length())
    {
      index = 0;
    }
    ToQueFromIndex(index);
    next();
  }
  void playPrevious()
  {

    index -= 1;
    if (index < 0)
    {
      index = current_album.length() - 1;
    }
    ToQueFromIndex(index);
    next();
  }
};

bool card_present;
bool PICC_IsAnyCardPresent()
{
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);
  mfrc522.PCD_WriteRegister(mfrc522.TxModeReg, 0x00);
  mfrc522.PCD_WriteRegister(mfrc522.RxModeReg, 0x00);
  mfrc522.PCD_WriteRegister(mfrc522.ModWidthReg, 0x26);
  MFRC522::StatusCode result = mfrc522.PICC_WakeupA(bufferATQA, &bufferSize);
  return (result == MFRC522::STATUS_OK || result == MFRC522::STATUS_COLLISION);
} // End PICC_IsAnyCardPresent()

Spotify spotify;
int last_pr = 1;
int last_pau = 1;
int last_ne = 1;
void setup()
{
  WiFiMulti WiFiMulti;
  Serial.begin(115200);

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(21, INPUT_PULLUP);
  pinMode(33, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);

  Serial.println();
  Serial.println();
  Serial.println();
  WiFi.mode(WIFI_STA);
  // WiFiMulti.addAP("River gast", "");
  WiFiMulti.addAP("s634a0208-slow", "llpu6h50");
  Serial.print("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED))
  {
    Serial.print(".");
  }
  Serial.println(" connected");
  spotify.setup();
  spotify.setupAlbums();
}

void handle_spotify()
{
  spotify.handleAutomaticNext();

  // handle buttons
  int prev = digitalRead(21);
  int pau = digitalRead(33);
  int ne = digitalRead(15);
  if (!prev)
  {
    Serial.println("Previous!");
    spotify.playPrevious();
  }
  if (!ne)
  {
    Serial.println("Next!");
    spotify.playNext();
  }
  if (!pau)
  {
    if (spotify.pause_play)
    {
      Serial.println("Pause!");
      spotify.Pause();
      StartTime = millis();
    }
    else
    {
      Serial.println("Play!");
      spotify.play();
      added_time += (millis() - StartTime);
    }
    spotify.pause_play = !spotify.pause_play;
  }
  last_pr = prev;
  last_pau = pau;
  last_ne = ne;
}

void loop()
{
  card_present = false;
  card_present = PICC_IsAnyCardPresent();
  if (!card_present)
  {
    card_present = PICC_IsAnyCardPresent();
  }

  if (!card_present)
  {
    last_hex = "";
  }

  if (last_hex == "" && !card_present)
  {
    spotify.Pause();
    Serial.println("Searching new card");
    last_hex = loop_until_ID();
    Serial.println(last_hex);
    for (int x = 0; x < spotify.every_album.length(); x++)
    {
      if (spotify.every_album[x]["hex"] == last_hex)
      {
        spotify.changeCurrentlyPlayingAlbum(spotify.fix(JSON.stringify(spotify.every_album[x]["albumUri"])));
        // spotify.changeToRaspotify();
        break;
      }
    }
  }
  if (last_hex != "" && card_present)
  {
    // Serial.println("handling spotify");
    handle_spotify();
  }
}
