#include <Arduino.h>
/* ヘッダファイルの読み込み */
#include <TinyGPS++.h>//GPSモジュール用
#include <time.h>//日付・時刻関係
#include <SPI.h>//SDカードモジュール用
#include <SD.h>//SDカードモジュール用
#include <Wire.h>//OLEDディスプレイ用
#include "ssd1306_extend.h"//OLEDディスプレイ用
#include "monospace_font.h"//OLEDディスプレイ用
#include <WiFi.h>// PlatformIOのWiFi.hライブラリをインストールすると上手く行かなかった

#define SW 5 //スイッチ
#define SD_CS 4 //SDカードモジュールのCSピン
const char* ssid = "AirPort52544";
const char* password = "6218455563710";
/* GPS/OLEDディスプレイ用のグローバル変数の宣言 */
TinyGPSPlus gps;
SSD1306_EX display(0x3c, 21, 22);//(I2Cのアドレス，SDAのピン番号，SCLのピン番号)
/* グローバル変数の宣言 */
int year, month, day, hour, minute, second, second_old;
float lat, lng, alt, speed;
boolean is_logging, is_valid;
String fname;

void setup() {
  // シリアルの初期化
  Serial.begin(115200);//シリアル通信用
  Serial.println("WiFi begin");
  WiFi.begin(ssid, password);
  Serial.print("WiFi connecting to: ");
  Serial.print(ssid);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.print(" connected.");
  Serial.println(WiFi.localIP());

  // スイッチのピンを入力用にする
  Serial2.begin(9600);//GPS用
  pinMode(SW, INPUT_PULLUP);
  // OLEDディスプレイの初期化
  display.init();
  display.flipScreenVertically();
  display.setFont(Monospaced_plain_8);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Hello");
  display.display();
  // SDカードモジュールの初期化
  if (SD.begin(SD_CS, SPI, 24000000, "/sd")) {
    Serial.println("SD_OK");
  }
  else {
    display.drawString(0, 0, "SD_Error");
    display.display();
    //while(1);//どうしてもSDカード読み込みエラーになる
  }
  // グローバル変数の初期化
  second_old = -1;
  is_logging = false;
  fname = "";
}


void read_gps(void) {
  struct tm t, *rt;
  time_t ti;

  // GPSから得た日時をt構造体に代入(時は9時間進める)
  t.tm_year = gps.date.year() - 1900;
  t.tm_mon = gps.date.month() - 1;
  t.tm_mday = gps.date.day();
  t.tm_hour = gps.time.hour() + 9;
  t.tm_min = gps.time.minute();
  t.tm_sec = gps.time.second();
  t.tm_wday = 0;
  t.tm_yday = 0;
  // 9時間進めた時刻を得る
  ti = mktime(&t);
  rt = localtime(&ti);
  // 日時を各変数に代入
  year = rt->tm_year + 1900;
  month = rt->tm_mon + 1;
  day = rt->tm_mday;
  hour = rt->tm_hour;
  minute = rt->tm_min;
  second = rt->tm_sec;
  // 緯度・経度・速度・標高と、緯度経度が有効かどうかを得る
  lat = gps.location.lat();
  lng = gps.location.lng();
  speed = gps.speed.kmph();
  alt = gps.altitude.meters();
  is_valid = gps.location.isValid();
}

void check_logging() {
  // スイッチがONかどうかを判断する
  if (digitalRead(SW) == LOW) {
    // ONならログを取る
    is_logging = true;
  }
  else {
    // OFFならログは取らない
    is_logging = false;
  }
}

void display_gps() {
  // ディスプレイを消去する
  display.clear();
  // 緯度を表示する
  display.drawString(0, 0, "LAT");
  display.drawDouble(40, 0, lat, 10, 6);
  // 経度を表示する
  display.drawString(0, 11, "LNG");
  display.drawDouble(40, 11, lng, 10, 6);
  // 標高を表示する
  display.drawString(0, 22, "ALT");
  display.drawDouble(60, 22, alt, 6, 1);
  // 速度を表示する
  display.drawString(0, 33, "SPEED");
  display.drawDouble(65, 33, speed, 5, 1);
  // 日付を表示する
  display.drawString(0, 44, "DATE");
  display.drawInt(40, 44, year, "%04d");
  display.drawString(60, 44, "/");
  display.drawInt(65, 44, month, "%02d");
  display.drawString(75, 44, "/");
  display.drawInt(80, 44, day, "%02d");
  // 時刻を表示する
  display.drawString(0, 55, "TIME");
  display.drawInt(50, 55, hour, "%02d");
  display.drawString(60, 55, ":");
  display.drawInt(65, 55, minute, "%02d");
  display.drawString(75, 55, ":");
  display.drawInt(80, 55, second, "%02d");
  // 各情報の間を線で区切る
  display.drawDotHLine(0, 10, 90);
  display.drawDotHLine(0, 21, 90);
  display.drawDotHLine(0, 32, 90);
  display.drawDotHLine(0, 43, 90);
  display.drawDotHLine(0, 54, 90);
  display.drawDotVLine(30, 0, 64);
  // 緯度経度が有効かどうかを表示する
  display.drawString(108, 0, "STAT");
  if (is_valid) {
    display.drawString(118, 11, "OK");
  }
  else {
    display.drawString(118, 11, "NG");
  }
  // ログ記録中かどうかを表示する
  display.drawString(113, 44, "LOG");
  if (is_logging) {
    display.drawCircle(123, 59, 4);
  }
  else {
    display.drawLine(119, 55, 128, 64);
    display.drawLine(119, 64, 128, 55);
  }
  // ここまでの内容を表示する
  display.display();
}

void log_gps(void) {
  boolean is_new;
  const char *mode;

  // ログ記録中かつ前回から1秒以上経過しているかどうかを判断する
  if (is_logging == true && second != second_old) {
    // ファイル名がまだ決まっていないかどうかを判断する
    if (fname.length() == 0) {
      // 現在の日付・時刻からファイル名を決める
      char buf[25];
      sprintf(buf, "/%04d%02d%02d%02d%02d%02d.CSV", year, month, day, hour, minute, second);
      fname = String(buf);
      // 新規作成モードにする
      is_new = true;
      mode = FILE_WRITE;
    }
    else {
      // 既存のファイルに追加するモードにする
      is_new = false;
      mode = FILE_APPEND;
    }
    // ファイルを開く
    File f = SD.open(fname, mode);
    if (f) {
      // 新規作成の場合は項目名をファイルに出力する
      if (is_new) {
        f.println("VALID,LAT,LNG,DATE,SPEED,LAT");
      }
      // 緯度経度が有効かどうかと、緯度・経度をファイルに出力する
      f.print(is_valid);
      f.print(",");
      f.print(lat, 6);
      f.print(",");
      f.print(lng, 6);
      f.print(",");
      // 日付をファイルに出力する
      f.print(year);
      f.print("/");
      f.print(month);
      f.print("/");
      f.print(day);
      f.print(" ");
      // 時刻をファイルに出力する
      f.print(hour);
      f.print(":");
      f.print(minute);
      f.print(":");
      f.print(second);
      f.print(",");
      // 速度をファイルに出力する
      f.print(speed);
      f.print(",");
      // 標高をファイルに出力する
      f.print(alt);
      f.println("");
      // ファイルを閉じる
      f.close();
    }
    else {
      // ファイルを開けなかった場合はエラーをシリアルモニタに出力する
      Serial.println(F("File open error"));
    }
  }
  // 記録中でない場合はファイル名を空にする
  else if (is_logging == false) {
    fname = "";
  }
}

void loop() {
  // GPSモジュールからデータを受信している間は繰り返す
  while (Serial2.available() > 0) {
    // GPSの情報を解析し終わったかどうかを判断する
    if (gps.encode(Serial2.read())) {
      // GPSの情報を読み込む
      read_gps();
      // スライドスイッチの状態を調べる
      check_logging();
      // GPSの情報をディスプレイに出力する
      display_gps();
      // GPSの情報をログファイルに記録する
      log_gps();
      // 秒を更新する
      second_old = second;
    }
  }
}
