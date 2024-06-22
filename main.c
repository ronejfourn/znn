#include "zm_util.h"
#include "zm_loss.h"
#include "zm_layers.h"

int main() {
    u32 xs_s[] = {4, 2};
    f32 xs_v[4][2] = {
        {0,0},
        {0,1},
        {1,0},
        {1,1},
    };
    zm_tensor xs = zm_tensor_create(zm_arraylen(xs_s), xs_s, xs_v, false);
    
    u32 ys_s[] = {4, 1};
    f32 ys_v[4][1] = {{0}, {1}, {1}, {0}};
    zm_tensor ys = zm_tensor_create(zm_arraylen(ys_s), ys_s, ys_v, false);
    
    zm_layer l[] = {
        zm_layer_linear(2, 2),
        zm_layer_ReLU(),
        zm_layer_linear(2, 1),
    };
    
    zm_sequential s = {l, zm_arraylen(l)};
    zm_loss m = zm_loss_mse();
    
    const u32 epochs = 1000;
    const f32 learning_rate = 0.05;
    zm_tensor *ypred;
    
    for (u32 i = 0; i < epochs; i ++) {
        ypred = zm_sequential_forward(&s, &xs);
        zm_tensor *loss = zm_loss_calc(&m, ypred, &ys);
        zm_tensor_backward(loss);
        zm_sequential_shd(&s, learning_rate);
        zm_sequential_zero_grad(&s);
    }

    zm_tensor_print_data(ypred);

    zm_sequential_destroy(s);
    zm_tensor_destroy(ys);
    zm_tensor_destroy(xs);
    zm_loss_destroy(m);

    return 0;
}
