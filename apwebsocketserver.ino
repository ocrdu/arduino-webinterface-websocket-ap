#include <WiFiNINA.h>
#include <WebSocketServer.h>
#include <Arduino_LSM6DS3.h>
#include <Base64.h>
#include <RTCZero.h>
#include "wifi_secrets.h"

#define DEBUG 1

#ifdef DEBUG
  #define Sprint(a) (Serial.print(a))
  #define Sprintln(a) (Serial.println(a))
#else
  #define Sprint(a)
  #define Sprintln(a)
#endif

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
IPAddress APIP(192, 168, 2, 1);
int APStatus = WL_IDLE_STATUS;
const int APChannel = 14;
const int webPort = 80;
const int socketPort = 8080;
WiFiServer webServer(webPort);
WiFiServer socketServer(socketPort);
WebSocketServer webSocketServer;
WiFiClient socketClient;
RTCZero rtc;
const int ledPin = 12;
boolean on = false;
float volume = 0;
int cowbell = 1000;
float zdata[] = {1,1,1,1,1};
int zdatasize = sizeof(zdata) / sizeof(zdata[0]);
float tdata[] = {20,20,20,20,20,20,20,20,20,20};
int tdatasize = sizeof(tdata) / sizeof(tdata[0]);
int ledState = 0;
unsigned long epoch;
unsigned long previousCowbellMillis = 0;
unsigned long previousGravityMillis = 0;
unsigned long previousTemperatureMillis = 0;
char interface_gz_base64[] = "H4sICHSkrV0CAFdlYnBhZ2VTZXJ2ZWRCeUFyZHVpbm8uaHRtbADsGmtzm0bwe2f6Hygdt83EvB8C28oMkizLbZ1KqdM07vQDghNQI1DhJGR3+t+7x50EiNiVU6XTR5QI7vZ1u3u7t3jR2WeD7/rXb8fnXIjn8YtPPzkjdy52k6DLo4QHCMedhcj1yxGMcYRj9OIywSibuR46kyiAYecIu5wXulmOcJdf4plg8RtcHCW3XIbiLp+HaYa9JeYiL014LszQrMv7LnZPorkbIGktEMTp1M2RqR87jtNznHPnHK7kfun0UjIcBnDpBwQ4gS/AN/jap3f1LVyv+4Xz5M9Ncan05pPR1bf2905v3nPe5zN6i7+fWNX84rVzlQ0TZ3At3ThOEUx26Z140vsaTMin14XzelTT+/uJc3WTOMnkclhcvB69mTqpJanFN+OxPXcGY+kmcO7ZPO//ci9t8O15TZ/ha2c0UK6d3q8jsFe+UD3n5Rtckwf6rmzcv76X3TvHyH/QnW/6YeD0i3Aa3D4f/ngZT4KrHSN6t87AWd/0ztfnlxP1x4Gf9gYz5Wbi3EnqW+dKsmNn8K00DYrnz7XJ7avwfH9/Dr3+pfMq6EX3sD+OQuUx+x1nLWmTm1dZNLxS9Iv9hUI83b+9JPrZVF5lP50nzvUr/albnyjPncnF7cVLKQR7R0xeJZ+sFwT7y1vM55f9S0/5Me9FzvcSk1f505281qTF8GI6ut1f6IXlTXq9XweDX3S6Py37By/H897V8O2PT/Dnd07v9fhN4cTWReC8nNXlwXxl3zj9kfNq4ox6T7B/9sur64F2fjNZQXy1/bkG/dfj4qn56UwmEzvr3WfFu+yPnPf59GA/zsdEnnUBSTum+1PKn8N85Dnv83EdRxmot4OXq1i+OJ/eTSepZV2QfF4oF6+9tw3i/fPJGa3D8nyIruNoOLfejO9+KOfh/c2s/wRR0r21Ws10KZ1qgXNp2p2edLEaB1I4WBVjO+hYI2u4mjlS1NGCWfHc1kc2Hn/XkxYDLZDs3NbH0jdjNJL8gRZKKjI7M+t2Nel2twUkx3dlpZmm/h33GwFOXe82yNJl4gteGqfZCff5uXFuDIenn37y+6efLI6hkk1RTIlnaYKFmTuP4rsTzskiNz7mcjfJhRxl0YxyiFB3EuThKE24fBVQxhXKcOS5seDGUZCccNMU43ROGXblFygKQkxoYv90C82je3TCKaK5Kqp1cJbGOWUtIh+HQCDLR6dcyESosrxYlzL8KF/ELmg9ixGFlJoIEUbz/ITzEKnGTHBeRNgLUf6Qg4oQuE7ri6q1NTugQMmXZj4C6jyNI59TQA8GEjLXj5awqEFgczcLokSI0Qx4hZJs4fp+lARA8DTlAf7LMsfR7E4gvgFoA5cuXC/CIEUWNWZpmqSzGTW34UWr7kSYMPKKsqZTkiaogaf72SSaxql3S7RgUtlGAYA5ALRixnrLLE8zIhdDtMRpgfzTnX3I0AK5mJCw4S7BIs0jEoCV/Q08CybiJDdKmsY953bNqPGRZytgXGbxV1/WHrYgzJ+v5/HpEs+s4yOtD/MjVYaYz0GHI21wpKqKqMAVoKWHGUyWKYz6ZAe4ilDRS9cUKgOAfBV2ZTSwaJJTihDjxZHmHKlD+F8UhVhoYpoFMAFOGW6gFGHSzkFBoh7OIHFnaTan/DlkJzpSLRHIOzrgywEsYm+5Fi4OAeFThrlqGKJhEHU0UZZ1T1A0XZQVoqmg6qpoKqWqlmhq1ZwNmDWaJtrWhqrCtgZ1WnkLFajwal4RI4GABcokNJRo3u7niiEqpk6EKKqoWqGgQX6onZWgwKZpIZ0BmWyKCiErAbpH1DBN0dZ0wdBFXSaokoPybWeEhmhNwYRWM02hQQlIQxbN0jpNEQ1FFWxNNHWC6hiipeiCoihiR1mBOYZqeYLaEWW7dL0hdgxN0A1Rtwi7roum0anmHUtUNk4zZDDUJkQyyCRusmXR7tibgVVBKHGH7hODmkLJV023tLIAeummISiWqKuqYKpwY0qYQqnCSmCq62C/TDQHWvDfxkJiqqlA9NQB1Oh7GuqzKI4FdoTRABQNismhCtwioZZYNDuGNGoz5GGSKpv0AtfplJHJsUBduZ2dqg0rWO0E1VRN1OxKKYCWE20my3WF2vC2ogoJHLXStp1j4MRyq0CXjwn2McE+cIJtY1aTO4prvzOc5YfD2ahHMgyC7ShfkfGXzxq19sQLkXeL/APW3IglDhWXJiUtzP915Xju4ixa1+sxW5PVZTIwYI91NjAfLdSaSEOCkH48Rz6eIx/6HDlIodZFo1moO7Zoy+pfL9Sy7Fma+gFqdS2oPybZxyT7wEn2txXrOPJR1myz6PKH7Xg80HQ5ZO+DmvVI88iQjw7WfSELrtJ4OUfHnOilxRTF8JzzcLujUpB1uZrtGeOoQUMfmxqWbCiiZLHEP+G7BepmbhKgnymZUKDpbQQrLhbIBYSHWCOnJqNjNDppzW6NCu0aToFLNaKOXuI4Spi0B1Q4OdksT9UXsmWSuNMYCTiDfd9Pw/Z+WUwFFoYkAFkofi4r5F8rHNUtx1rIQ9dPC8rFvsBXfo45YqK8Bfrk306YQtuUpuG+NuNwOZ8+wVRtsa5M1fRHbC0/LVu1Q9qaBVP3K9UwjrnqojyrBYiA0wUkN9H6YYfM03uBTLb7/q/d2soStq3/vp0jp0ngLoMntL0148MWgUMcvK2jvzpxfTRzlzGu275v17qqg8Zha0RA1CA1AjN9WosDHXuXI21e5pDx9vcFZ+TVzgtG4kcrLvK7fPVKhue82M3zJqgU1OXZOjo5y/kXZxJwbwSFWUske/vCBNYAlLJOy16oVLQMwGh3qGsvJRhDE8Z4GFeZjlyZjnzZT4A04emqTETizgHXEFhNYeNI5fG5NOmHkMFAid349ivCdMyxBsWznTVpvZ2lWSWnWrBEsnWaMHBpOagJqzuZzdruYw9gTGY131C2aZUd2kd8Vh5bVH/6gMIcxmY7gtgm89w8Srq8DHd33eUVGUYrN14igDV8Wq5Udykle8Sh71yW+e+HEvcnXmx7Q30vb7CnNOqO7fRv90dt3bZD+hT5nnFFj3smuZqmiZ+liy5Prl+hFUrwMYfDKH9GUW6QrhAoVj6sDrY0lQmtJZTmEjxHhATEJeCJbIk2cnPsZuAcMmzLJFLzVUClMjlMKpvSM7nLm+B9dmayCWv9wcaIoAvr5MHWcDJn2Ap8AUp6dyCL9O1OJKnq2EmkX0e6jZC+AUf/jOrypOYCwIsyL0acB8I02VysQaU7MtTJEHykdgwYsRIEGNHgmQiBKUvwEkhug3fFy5V4uSme/DHY5dlDCi/twWfIFR973U/4yJM7t1ZKUvCIygZ3ADHIXSV3meq77VdC0qTYxegr0ujhgIF8n/FPkSYF75aoP1me8ag8syMeWKItH1aeoqiHVlE4/L4If2Fj2HePIFWNVpDuZo/MsmdDZZYfvvY7FRBZ/UCFr35K0uX1ciGM1lhwEy8kZ+088v0YgUUEypGDXAH9iVa2yb+QzyQCr7CqbQOWUGn8C2UXq9t6xatSLLU8Qx6VbquiYpREuqyJFkjLCFiG+x29MztVxRBNuzrVOqpokTkzezab8Y2jaddPCvVSdXb6kRvz79hZrRHPizS+C9KkxpcgRDzELdIowTkoZm0ZOEOFu1qOt5pZMvn3uHIq2cQ9Y6J93j0uW6Oyie+pFWW55Tm6AHW9YYmGaqjKfmFjPBw2wXaToVzs83hSulQ9dHXEzeqI/xnVsfM/r46afNgz2Dxw5Tl4JVMPLFA4tAcFU/6fVDGdnXRqh38h6Js61q5yig54jeHb/KqsAl7dxQOc4hXNALzSwoM3qjrZxto17jazblrV4mobbduV7toD3Mx0Xf7HVGH8nlUY/yeqMP6QVfiLz9dT+bS/Ry1uzh7pd7H+2KbJV/b4aPHdJaFtQb3WGDXlI0bLqBcb2upnzYporAqgeoXyRZrkKP8imeaL01mWzunIyfxllKQnZ9KiLopY6GbI3bS4UAa1fI7y3C3/kI/8Fow+FLTBWVrkNNa9NIYRZHzTJNqPZ13brLSsfOfDAS9y/TSJ78DXG4WaDt7fU82+sLG/596gaQ69WoSpv7zQxRhlf+KvYsNEXVVNmZcqyL4O2tcZ1bhpGKSdm/3R3rX1Og3D4Hck/kMoEmxwGnJtU2BIwJhAAh6AByTgYWzlMGk3bR1Xnf+OnWRruw5WRoGXIsSaxnYcJ46/Omm5DYfjIcuJczDX/+56tJosMyhdvvRpuCJuAEmPBJ/XgMgCchNWqTnsDVDIWQ8xrUs/LtYZdhSqgtu4ANwK7jhm1ylgnm+mU3/Pz4VXk1nq74wgw5OOgerDcLqGm9Dyh83cHez32eOXVlCn6/Pmi9FmBsCUnqfZo2mKlw++Phl3irnmLp3A9erxq2dPQfL1vOtuk+HqYHBfMwYjinDWrzG4F1aGqfmSrPLlWKg99CoU/K2HXfEoB4E5MOOC8JgqqUcho4kwnLCQ08gQRmOtjLvGfzjUECSJhK3jCd6O48jW+mtLECvC7M3Q0il3vSUIrQxp60QUOtH2x19/m+HGv1JhTBPNxCgUNJFChZIqozTcFVyYUGMpDi2p3JYU1ZJhG4YaDgoKCj0WjkgRT8SoNAlToKqKZKyxLGKjQUuuFfaby4jFUNRKxlAUDHtkD0REyKRFYm/rSKMhmBAmL0qukAquIqGiKERpHCuMNDKybRtQS3PNsV+JUCSi3AjUPWYx2JFRFpltSVJjuAR5MdWao6ljrqUjigkSKYGNc6dpxDj8aGFvyoRHrmGVl4RKpFUPcAR2nRmV2F5IEbpOOLs4TRMvDUaGOcuA2s4w32be9lDmjCcjGB8hNRo/ogIGDHRVQiehAUGxgJ4kYN6y8Q0WJZiiaHtormj7SCZF20eRKNpeJ6poe6a96UEjZ3pVMn3s5mwE8wZsLw1YkEMToQRSE8FIxCDCl3CUEhCnKdgG+bhSBklYEhFPgpY3MtdAC2jBNeQtn5eEZqJseS5sHyJTtLyqWj5OsITzyNs+FJQlCv5VBn402I7FoTcwmjuSvvRtxoiCG8KMsMdKga5QFceGo34a6LF7EqekVIkpDQ/jJdfgwk+AyA4P43Z4ZBTZ4dGi5Bqx8pPKl/Y8A93OjTWqoQTjea+NVDEWk8TE6MDaKOx2lEiwutRyW1qjF8UG7QQ2tHVCE1c3qg6MMSRvQom8IDTbeURkx0WIokfEqjQuVY8QobMH/AitcWopsHQogcBEoTesm0W+9A0D7mQZrjYYCDC3sRiPtwDTRwRXLJPAuj3EE75ZSiDcrybvN1n63Eb24hnEIK98Zfd4Xj97GpDxBiAj9Aewj3uF6CFsdUMQgRiafpjMJ1nqd3Aw6t9xj30YJDygJA99SJufE0opxunrNkKSQnBNPxPAJT5GuoDdLRHh+1fLdA6028C6DaWkGn0JaSbCMnZf6QdthG0jbBth2wjbRtg2wrqIcDDC3tqLd+nYhrpj4cifiulSe/Shfhzzpz9OYPTHJE7g3J5FAlYbKKnHDcAf8OAItz+OcSKzTeadxls8WLQT4E62Ib/NDKaroKa9Txfg7X5MwsUe7PHZhiLyme2gT56KQII+4Ds6X3zudL0u6zTDqsUm61RhEyGTD6ST85CwJO4e5kUZuXaNXHEAK2csYi7cd/OtNQW7BrHWUrewq4VdLexqYVcLu1rY5SLCr2FXf7IeHUJev4e+6kc0DwhO4/VYoC5zfRjGqNwCkVpg7HQROSQ7XYJNG+3hgeC8Fm92kNfvaNaRUDqmhb1Is/vb7FfHpsWCM7LbEO7W0qlxifkRtIq83V44CM13w8+IZO6fbk2d/3oLv4bfhZf5gvpe14Ag74L1JXlEPpou1ukOYVu8bn/OiGJgkyNPAflGaWnq3oRm74YE9yZnFD/dABfB23nwO9Jg23Mxnb5aLEnvdxgeW+jsG7JbnkDcRxV6XhcKe/mTrBPcduPtHxy2ZG/YO9LrERjmwD4h1F1xt1+06BVEcScKF0Joq76sxXyEb6Tkw3JBUniePaSnn0BO17pzza41oGlB0Tv1uf27DHWU85Oylnae9lT1PPvv6Xe+Gn6CRT7Xz00ZfxtUWOLHWAfTxTArjipKzqndotaHJefFIrMb8DgJtjJuEHuyuwsegIfQd5yozgHOez3CNdDhg+qB6rs9IpgV972xkCDFw74UfoLuGeyQCkSVmj/cf6Wbi1mDAYM/tRW8RySvoaHk/0DF5kZpMOjH0EJpoW422uJqXbUUrtyVGHkCDvL+QLPFYPIlHXcEegQCHMHEhzvnwXFnzdLZMl0Ns80q3XfYQtXPnZaEJL5T4jrouEVZN4hENblhJb/NanhFdsQrCh3N6szh7NgcvjiiIawsQth1JTu8rmhssDFY+ItV5bD9hGiyfXIVH8P6D2orcK9pA1SWBBigZsErOmzWhMMeevgo+tTOaXnJaf3TSQ3XdYnYeiilfECxDCtLmKCEKetLrEDL2lxVfHlxKMOdrlaLVXVnvwq5DzHbuj3m5k4AtKnoNhXdpqLbVHSbim5T0X+eij4lDX1qCvrk9HMTqec/SDv/ecr5lHTzn6eam00zN5tibuqBt5r8bRKdV6U3m1ZuJKXcTDq5cC6j9HrIGeGMsR3QtVi3RNBxp2L9+yvD8fgRvt78dLLOUpianeB9CkZNN3N4lh8HZyVQfBQ1X2CzVch90S2/z1L5IIsTna2+7qF2ePIYd3Zv1cCzl6c/PUMe3iMopirzv2TLL8hoCDOQdLYm2H+kgYp8JIsmLH4PxvOmn+hyleJg9t23ttD2ZTb7tnuJA5Pzr6wnpSt0sD6U/SIZnCFBNlxBl+hkfECYax7Ipr9UwWV+sCXSq7R5Xm7T0oNAOlwuYfAffpxMx52fGRcFdbvFj3Tt3tvCgv82191b9r8K+wGyLbAzOmwAAA==";

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
  #endif
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledPin, OUTPUT);
  rtc.begin();
  IMU.begin();
  WiFiConnect();
}

void loop() {

  if (APStatus != WiFi.status()) {
    APStatus = WiFi.status();
    if (APStatus == WL_AP_CONNECTED) {
      Sprintln("\n--A device connected to the AP");
    } else {
      Sprintln("\n--A device disconnected from the AP");
    }  
  }

  WiFiClient webClient = webServer.available();
  if (webClient.connected()) {
    Sprint("\n--New client: "); Sprint(webClient.remoteIP()); Sprint(":"); Sprintln(webClient.remotePort());
    String header = "";
    unsigned long requestStartMillis = millis();
    while (webClient.available()) {
      unsigned long requestCurrentMillis = millis();
      char c = webClient.read();
      if (c != '\r') {
        header += c;
      }
      if ((requestCurrentMillis - requestStartMillis) > 5000) {
        Sprintln("--Request is taking too long");
        webClient.println("HTTP/1.1 408 Request Timeout\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n408 Request Timeout\n");
        webClient.stop();
        Sprintln("--Client disconnected");
      }
      if (header.substring(0,1) != "G" && header.substring(0,2) != "GE" && header.substring(0,3) != "GET") {
        Sprintln("--Wrong method in header");
        webClient.println("HTTP/1.1 405 Method Not Allowed\nAllow: GET\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n405 Method not allowed; GET only\n");
        webClient.stop();
        Sprintln("--Client disconnected");
      }
      if (header.indexOf("Upgrade") > -1 && header.indexOf("ebsocket") > -1) {
        Sprintln("--Websocket upgrade requested on webserver port");
        webClient.println("HTTP/1.1 400 Bad Request\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n400 Bad Request; websocket not available on this port\n");
        webClient.stop();
        Sprintln("--Client disconnected");
      }
      if (header.indexOf("event-stream") > -1) {
        Sprintln("--SSE content type request in header");
        webClient.println("HTTP/1.1 406 Not Acceptable\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n406 Not Acceptable; SSE not available\n");
        webClient.stop();
        Sprintln("--Client disconnected");
      }
      if (header.length() > 512) {
        Sprintln("--Header too long");
        webClient.println("HTTP/1.1 431 Request Header Fields Too Large\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n431 Request header fields too large\n");
        webClient.stop();
        Sprintln("--Client disconnected");
      }
      if (header.substring(header.length() - 2) == "\n\n") {
        Sprint(header.substring(0, header.length() - 1));
        if (header.indexOf("GET / HTTP") > -1) {
          sendBase64Page(interface_gz_base64, webClient, 1024);
          Sprintln("--Interface webpage sent");
        } else if (header.indexOf("GET /author HTTP") > -1) {
          webClient.println("HTTP/1.1 200 OK\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\nOscar den Uijl, oscar@den-uijl.nl\n");
          Sprintln("--Email address sent");
        } else if (header.indexOf("GET /coffee HTTP") > -1) {
          webClient.println("HTTP/1.1 418 I'm a teapot\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n418 I'm a teapot\n");
          Sprintln("--Coffee request denied");
        } else {
          webClient.println("HTTP/1.1 404 Not Found\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n404 Not Found\n");
          Sprintln("--Page not found");
        }
        webClient.stop();
        Sprintln("--Client disconnected");
      }
    }
  }

  if (!socketClient.connected()) {
    socketClient = socketServer.available();
    if (socketClient.connected() && webSocketServer.handshake(socketClient)) {
      Sprint("\n--Websocket connected to: "); Sprint(socketClient.remoteIP()); Sprint(":"); Sprintln(socketClient.remotePort());
      if (on) {
        webSocketServer.sendData("sw:true");
      } else {
        webSocketServer.sendData("sw:false");
      }
      webSocketServer.sendData("volume:" + (String)round((volume*100)/255));
      webSocketServer.sendData("cowbell:" + (String)((1000-cowbell)/10));
      Sprintln("--Settings sent");
    } else {
      //Sprintln("\n--Couldn't connect websocket");
      socketClient.stop();
      delay(100);
    }
  }

  if (socketClient.connected()) {
    String data = webSocketServer.getData();
    if (data.length() > 0) {
      String name = data.substring(0, data.indexOf(":"));
      String value = data.substring(data.indexOf(":") + 1);
      boolean goodSettings = true;
      if (name == "switch") {
        on = (value == "true");
      } else if (name == "volume") {
        volume = (value.toFloat()*255)/100;
        if (cowbell == 1000) {
          analogWrite(ledPin, (int)round(volume));
        }
      } else if (name == "cowbell") {
        cowbell = 1000 - (value.toInt()*10);
      } else {
        goodSettings = false;
        webSocketServer.sendData("message:Bad data; ignored");
      }
      if (goodSettings) {
        webSocketServer.sendData("message:" + name + " set to " + value);
      }
    }
  }

  unsigned long currentCowbellMillis = millis();
  if ((currentCowbellMillis - previousCowbellMillis) > cowbell) {
    previousCowbellMillis = currentCowbellMillis;
    if (on) {
      if (cowbell == 1000) {
        analogWrite(ledPin, (int) round(volume));
      } else {
        if (ledState == 0) {
          ledState = 1;
          analogWrite(ledPin, (int) round(volume));
        } else {
          ledState = 0;
          analogWrite(ledPin, 0);
        }
      }
    } else {
      analogWrite(ledPin, 0);
    }
  }

  float t, tsum = 0;
  if (IMU.temperatureAvailable()) {
    IMU.readTemperature(t);
    for (int i = 0; i < tdatasize - 1; i++) {
      tdata[i] = tdata[i+1];
    }
    tdata[tdatasize-1] = t;
    for (int i = 0; i < tdatasize; i++) {
      tsum += tdata[i];
    }
    if (socketClient.connected()) {
      unsigned long currentTemperatureMillis = millis();
      if ((currentTemperatureMillis - previousTemperatureMillis) > 5000) {
        previousTemperatureMillis = currentTemperatureMillis;
        webSocketServer.sendData("temperature:" + String(tsum/tdatasize, 3));
      }
    }
  }
  
  float x, y, z, zsum = 0;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    if (x > 0.2 || x < -0.2 || y > 0.2 || y < -0.2 || z > 1.2 || z < 0.8) {
      analogWrite(ledPin, 0);
      Sprintln("\n--TILT!!");
      if (on) {
        on = false;
        if (socketClient.connected()) {
          webSocketServer.sendData("sw:false");
        }
      }
    }
    for (int i = 0; i < zdatasize - 1; i++) {
      zdata[i] = zdata[i+1];
    }
    zdata[zdatasize-1] = z;
    for (int i = 0; i < zdatasize; i++) {
      zsum += zdata[i];
    }
    if (socketClient.connected()) {
      unsigned long currentGravityMillis = millis();
      if ((currentGravityMillis - previousGravityMillis) > 2000) {
        previousGravityMillis = currentGravityMillis;
        webSocketServer.sendData("gravity:" + String(zsum/zdatasize, 3));
      }
    }
  }

}

void WiFiConnect() {
  Sprintln("--Setting up access point " + (String)ssid + " ...");
  WiFi.setHostname("nano");
  WiFi.config(APIP, APIP, APIP, IPAddress(255,255,255,0));
  APStatus = WiFi.beginAP(ssid, pass, APChannel);
  if (APStatus != WL_AP_LISTENING) {
    Sprintln("--Setting up access point " + (String)ssid + " failed");
    while (true);
  }
  webServer.begin();
  socketServer.begin();
  #ifdef DEBUG
  printWifiStatus();
  #endif
  WiFi.lowPowerMode();
  Sprintln("--Access point " + (String)ssid + " set up");
  digitalWrite(LED_BUILTIN, HIGH);
}

void sendBase64Page(char base64Page[], WiFiClient client, int packetSize) {
  const int base64Page_length = strlen(base64Page);
  const int page_length = base64_dec_len(base64Page, base64Page_length);
  char page[page_length];
  int done = 0;
  base64_decode(page, base64Page, base64Page_length);
  client.println("HTTP/1.1 200 OK\nConnection: close\nContent-Type: text/html");
  if (page[0] == 0x1f && page[1] == 0x8b) {
    client.println("Content-Encoding: gzip");
  }
  client.println("");
  while (page_length > done) {
    if (page_length - done < packetSize) {
      packetSize = page_length - done;
    }
    client.write(page + done, packetSize * sizeof(char));
    done = done + packetSize;
  }
}

#ifdef DEBUG
void printWifiStatus() {
  Sprint("Access point SSID: "); Sprintln(WiFi.SSID());
  Sprint("Signal strength (RSSI): "); Sprint(WiFi.RSSI()); Sprintln(" dBm");
  Sprint("IP address: "); Sprintln(WiFi.localIP());
  Sprint("Gateway: "); Sprintln(WiFi.gatewayIP());
  Sprint("Netmask: "); Sprintln(WiFi.subnetMask());
  Sprint("Webserver is at http://"); Sprint(WiFi.localIP()); Sprintln(":" + (String)webPort + "/");
  Sprint("Websocket is at http://"); Sprint(WiFi.localIP()); Sprintln(":" + (String)socketPort + "/");
}
#endif
