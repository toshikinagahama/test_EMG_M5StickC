#include <M5StickC.h>
#define SAMPLING_FREQUENCY 1000
#define N 5
#define N_BUF 2000 // 2秒間
unsigned long time_s = 0;
unsigned long time_e = 0;
unsigned int sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
double ori[N] = {0.0};
double fil[N] = {0.0};
// 過去データバッファ
double buf[N_BUF] = {0.0};
double th1 = 0.6;
bool is_return_zero = false;
unsigned long peak_cnt = 0;
unsigned long peak_cnt_last = 0;
double max_2s = 0.0; // 2秒間の最大値
// pass 8-20Hz, cutoff 5-35Hz, gain 10-20Hz
double a[N] = {
    1.0,
    -3.92599874,
    5.79268939,
    -3.80692796,
    0.940276,
};
double b[N] = {0.00045981, 0., -0.00091962, 0., 0.00045981};
unsigned long cnt = 0;

void setup()
{
  M5.begin();
  Serial.begin(115200);
  pinMode(33, INPUT);

  time_s = micros();
}

void loop()
{
  int value = analogRead(33);
  // フィルタ
  ori[cnt % N] = value;
  fil[cnt % N] = 0.0;
  double tmp = 0.0;
  for (int i = 0; i < N; i++)
  {
    tmp += b[i] * ori[(cnt - i) % N] - a[i] * fil[(cnt - i) % N];
  }
  if (tmp >= 100000)
  {
    tmp = 0.0;
    for (int i = 0; i < N; i++)
      fil[i] = 0.0;
  }
  fil[cnt % N] = tmp;
  buf[cnt % N_BUF] = tmp;

  // ピーク検出
  if (buf[cnt % N_BUF] >= max_2s * th1)
  {
    if (is_return_zero)
    {
      if (60000.0 / (double)(cnt - peak_cnt) <= 150.0)
      {
        peak_cnt_last = peak_cnt;
        peak_cnt = cnt; // 本当はピークまで見たいけどとりあえず
        double hr = 60000.0 / (double)(peak_cnt - peak_cnt_last);
        Serial.println(hr);
        is_return_zero = false;
      }
    }
  }

  if (is_return_zero)
  {
    max_2s = 0.0;
    for (int i = 0; i < N_BUF; i = i + 10)
    {
      // 10ms間隔で十分
      if (buf[i] >= max_2s)
      {
        max_2s = buf[i];
      }
    }
  }

  if (buf[cnt % N_BUF] < 0)
  {
    is_return_zero = true;
  }

  // if (cnt % 5 == 0)
  // {
  //   // Serial.println(value);
  //   if (tmp < 0)
  //   {
  //     Serial.println(0);
  //   }
  //   else
  //   {
  //     Serial.println(tmp);
  //   }
  // }

  while ((time_e - time_s) < sampling_period_us)
  {
    time_e = micros();
  }
  time_s = time_e;
  cnt++;
}
