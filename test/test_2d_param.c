// Test 2D array parameter
int f(int arr[][3]) {
  return arr[1][2];
}

int main() {
  int arr[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return f(arr);
}
