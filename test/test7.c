int seed;

int nextInt() {
    seed = seed * 1103515245 + 12345;
    if (seed < 0) {
        seed = -seed;
    }
    return seed;
}

float nextFloat() {
    int x;
    x = nextInt();
    return x / 10000.0;
}

int absInt(int x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

float absFloat(float x) {
    if (x < 0.0) {
        return -x;
    }
    return x;
}

int minInt(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

float localEnergy(float a[20][20], int x, int y, int depth) {
    float v;
    float sum;
    int nx;
    int ny;

    v = a[x][y];

    if (depth <= 0) {
        return v;
    }

    sum = v * 0.5;

    nx = x - 1;
    ny = y;

    if (nx >= 0) {
        sum = sum + localEnergy(a, nx, ny, depth - 1) * 0.1;
    }

    nx = x + 1;

    if (nx < 20) {
        sum = sum + localEnergy(a, nx, ny, depth - 1) * 0.1;
    }

    nx = x;
    ny = y - 1;

    if (ny >= 0) {
        sum = sum + localEnergy(a, nx, ny, depth - 1) * 0.1;
    }

    ny = y + 1;

    if (ny < 20) {
        sum = sum + localEnergy(a, nx, ny, depth - 1) * 0.1;
    }

    return sum;
}

void printMap(float a[20][20]) {
    int i;
    int j;
    int v;

    i = 0;

    while (i < 20) {
        j = 0;

        while (j < 20) {
            v = a[i][j];

            if (v < 0) {
                v = 0;
            }

            if (v > 9) {
                v = 9;
            }

            putch(v + 48);

            j = j + 1;
        }

        putch(10);
        i = i + 1;
    }
}

int main() {
    float world[20][20];
    float buffer[20][20];

    int i;
    int j;
    int step;

    float total;
    float avg;

    int cx;
    int cy;

    float e;

    seed = getint();

    i = 0;

    while (i < 20) {
        j = 0;

        while (j < 20) {

            world[i][j] = nextFloat();

            if (world[i][j] > 50.0) {
                world[i][j] = world[i][j] / 10.0;
            }

            if ((i + j) / 2 * 2 == i + j) {
                world[i][j] = world[i][j] + 3;
            }

            j = j + 1;
        }

        i = i + 1;
    }

    step = 0;

    while (step < 15) {

        i = 1;

        while (i < 19) {

            j = 1;

            while (j < 19) {

                buffer[i][j] =
                    world[i][j] * 0.4 +
                    world[i - 1][j] * 0.15 +
                    world[i + 1][j] * 0.15 +
                    world[i][j - 1] * 0.15 +
                    world[i][j + 1] * 0.15;

                if (buffer[i][j] > 30) {
                    buffer[i][j] = buffer[i][j] * 0.7;
                }

                if (buffer[i][j] < 1.0) {
                    buffer[i][j] = buffer[i][j] + nextInt() % 5;
                }

                j = j + 1;
            }

            i = i + 1;
        }

        i = 1;

        while (i < 19) {

            j = 1;

            while (j < 19) {

                world[i][j] = buffer[i][j];

                j = j + 1;
            }

            i = i + 1;
        }

        cx = nextInt() % 20;
        cy = nextInt() % 20;

        if (cx < 0) {
            cx = -cx;
        }

        if (cy < 0) {
            cy = -cy;
        }

        e = localEnergy(world, cx, cy, 2);

        if (e > 40) {
            world[cx][cy] = world[cx][cy] * 0.5;
        } else {
            world[cx][cy] = world[cx][cy] + 2;
        }

        step = step + 1;
    }

    total = 0.0;

    i = 0;

    while (i < 20) {

        j = 0;

        while (j < 20) {

            total = total + world[i][j];

            j = j + 1;
        }

        i = i + 1;
    }

    avg = total / 400;

    printMap(world);

    putfloat(avg);
    putch(10);

    putfloat(localEnergy(world, 10, 10, 3));
    putch(10);

    putint(seed);
    putch(10);

    return 0;
}