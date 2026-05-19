void init(float arr[][10][10]) {
  int i = 0;
  while (i < 10) {
    int j = 0;
    while (j < 10) {
      int k = 0;
      while (k < 10) {
        arr[i][j][k] = i * 100 + j * 10 + k;
        k = k + 1;
      }
      j = j + 1;
    }
    i = i + 1;
  }
}

int f1(float a0[], float a1[], float a2[], float a3[], float a4[], float a5[], float a6[],
       float a7[], float a8[], float a9[]) {
  return a0[0] + a1[1] + a2[2] + a3[3] + a4[4] + a5[5] + a6[6] + a7[7] + a8[8] +
         a9[9];
}

int f2(float a0[][10], float a1[], float a2, float a3[], float a4[], float a5[][10][10],
       float a6[], float a7[], float a8, float a9[][10]) {
  return a0[0][9] + a1[1] + a2 + a3[3] + a4[4] + a5[5][5][5] + a6[6] + a7[7] +
         a8 + a9[9][8];
}

int main() {
  float arr[10][10][10], sum = 0;
  init(arr);
  sum = sum + f1(arr[0][0], arr[1][1], arr[2][2], arr[3][3], arr[4][4],
                 arr[5][5], arr[6][6], arr[7][7], arr[8][8], arr[9][9]);
  sum = sum + f2(arr[0], arr[1][1], arr[2][2][2], arr[3][3], arr[4][4], arr,
                 arr[6][6], arr[7][7], arr[8][8][8], arr[9]);
  putfloat(sum);
  putch(10);
  return 0;
}
