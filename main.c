#include "zm_util.h"
#include "zm_loss.h"
#include "zm_layers.h"
#include "zm_random.h"
#include "zm_optimizers.h"

#define EPOCHS 5
#define BATCH_SIZE 64
#define LEARNING_RATE 1e-3

u32 tou32(u8 *buf) {
    return (((u32)buf[0]) << 24) | (((u32)buf[1]) << 16) | (((u32)buf[2]) << 8) | (((u32)buf[3]));
}

typedef struct {
    zm_tensor data;
    zm_tensor label;
    u32 i;
} dataset;

void new_epoch(dataset *d) {
    d->i = 0;
}

bool get_batch(dataset *d, zm_tensor *X, zm_tensor *Y) {
    if (d->i + BATCH_SIZE > d->data.shape[0]) return false;
    zm_tensor_view x = zm_tensor_get_range(&d->data, d->i, d->i + BATCH_SIZE);
    zm_tensor_view y = zm_tensor_get_range(&d->label, d->i, d->i + BATCH_SIZE);
    *X = zm_tensor_from_view(&x);
    *Y = zm_tensor_from_view(&y);
    X->shape[0] = BATCH_SIZE;
    Y->shape[0] = BATCH_SIZE;
    d->i += BATCH_SIZE;
    return true;
}

dataset load_mnist(const char *ipath, const char *lpath) {
    FILE *ifptr = fopen(ipath, "rb");
    FILE *lfptr = fopen(lpath, "rb");

    u8 buf[4];
    int p;
    p = fread(buf, 1, 4, ifptr);
    assert(tou32(buf) == 2051);

    p = fread(buf, 1, 4, lfptr);
    assert(tou32(buf) == 2049);

    p = fread(buf, 1, 4, ifptr);
    p = fread(buf, 1, 4, lfptr);
    u32 icount = tou32(buf);
    u32 lcount = tou32(buf);
    assert(icount == lcount);

    p = fread(buf, 1, 4, ifptr);
    p = fread(buf, 1, 4, ifptr);

    dataset d = {0};
    d.data = zm_tensor_create(icount, 28, 28);
    d.label = zm_tensor_zeros(icount, 10);

    u8 buf2[28*28];
    for (u32 i = 0; i < icount; i ++) {
        p = fread(buf2, 28, 28, ifptr);
        for (u32 j = 0; j < 28 * 28; j ++)
            d.data.data[i * 28 * 28 + j] = buf2[j] / 255.0;

        p = fread(buf, 1, 1, lfptr);
        d.label.data[i * 10 + buf[0]] = 1;
    }

    fclose(ifptr);
    fclose(lfptr);
    return d;
}

bool check(f32 *p, f32 *y) {
    u32 s = 0;
    for (u32 j = 0; j < BATCH_SIZE; j ++) {
        u32 mp = 0, my = 0;
        for (u32 i = 1; i < 10; i ++) {
            mp = p[j * 10 + i] > p[j * 10 + mp] ? i : mp;
            my = y[j * 10 + i] > y[j * 10 + my] ? i : my;
        }
        s += mp == my;
    }
    return s;
}

int main() {
    dataset train = load_mnist("./res/train-images.idx3-ubyte", "./res/train-labels.idx1-ubyte");
    dataset test = load_mnist("./res/t10k-images.idx3-ubyte", "./res/t10k-labels.idx1-ubyte");
    
#if 0

    FILE *gnuplot = popen("gnuplot", "w");
    if (!gnuplot) return 1;
    
    fprintf(gnuplot, "plot '-' u 1:2:3 w image \n");
    
    for (int i = 0; i < 28; i++)
        for (int j = 0; j < 28; j++) {
            fprintf(gnuplot, "%d %d %f\n", j + 1, i + 1, train.data.data[i * 28 + j]);
            fprintf(stdout, "%d %d %f\n", j + 1, i + 1, train.data.data[i * 28 + j]);
        }
    
    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
    getc(stdin);
#endif
    
    zm_layer l[] = {
        zm_layer_flatten(),
        zm_layer_linear(28*28, 512),
        zm_layer_ReLU(),
        zm_layer_linear(512, 512),
        zm_layer_ReLU(),
        zm_layer_linear(512, 10),
    };
    
    zm_sequential s = zm_sequential_create(l, zm_arraylen(l));
    zm_loss mse = zm_loss_mse();
    zm_optimizer sgd = zm_optimizer_SGD(s.parameters, s.n_params, LEARNING_RATE);
    
    zm_tensor x, y;
    for (u32 epoch = 0; epoch < EPOCHS; epoch++) {
        new_epoch(&train);
        new_epoch(&test);
    
        printf("   Epoch %d\n", epoch + 1);
        printf("----------------------------\n");
        for (u32 batch = 0; get_batch(&train, &x, &y); batch++) {
            zm_tensor *pred = zm_sequential_forward(&s, &x);
            zm_tensor *loss = zm_loss_calc(&mse, pred, &y);
    
            zm_tensor_backward(loss);
            zm_optimizer_step(&sgd);
            zm_optimizer_zero_grad(&sgd);
    
            if (batch % 100 == 0) {
                printf("[%5d/%5d] loss: %f\n", 
                        batch * BATCH_SIZE + BATCH_SIZE, train.data.shape[0], 
                        loss->data[0]);
            }
            zm_tensor_destroy(x);
            zm_tensor_destroy(y);
        }
    
        f32 test_loss = 0, correct = 0, count = 0;
        while (get_batch(&test, &x, &y)) {
            zm_tensor *pred = zm_sequential_forward(&s, &x);
            zm_tensor *loss = zm_loss_calc(&mse, pred, &y);
            test_loss += loss->data[0];
            correct += check(pred->data, y.data);
            count ++;
        }
            
        printf("accuracy: %5.3f%% avg. loss: %f\n", correct / (BATCH_SIZE * count) * 100, test_loss / count);
    }
    
    zm_sequential_destroy(s);
    return 0;
}
