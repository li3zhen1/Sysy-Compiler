int main() {
  int i = 0;
  while (i < 10 || i >50) {
    if (i == 5) {
      break;
    }
    i = i + 1;
  }
  return i;
}
