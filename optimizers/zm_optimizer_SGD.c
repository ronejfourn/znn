#include "../zm_optimizers.h"

ZM_OPTIMIZER_STEP_FXN(zm_optimizer_step_SGD) {
    for (u32 i = 0; i < this->n_params; i ++) {
        u32 S = this->parameters[i]->size;
        u32 L = S - (S & ~3);
        for (u32 j = 0; j < L; j ++)
            this->parameters[i]->data[j + 0] -= this->learning_rate * this->parameters[i]->grad[j + 0];
        for (u32 j = L; j < S; j += 4) {
            this->parameters[i]->data[j + 0] -= this->learning_rate * this->parameters[i]->grad[j + 0];
            this->parameters[i]->data[j + 1] -= this->learning_rate * this->parameters[i]->grad[j + 1];
            this->parameters[i]->data[j + 2] -= this->learning_rate * this->parameters[i]->grad[j + 2];
            this->parameters[i]->data[j + 3] -= this->learning_rate * this->parameters[i]->grad[j + 3];
        }
    }
}

zm_optimizer zm_optimizer_SGD(zm_tensor **params, u32 n_params, f32 learning_rate) {
    zm_optimizer s = {params, n_params, learning_rate, zm_optimizer_step_SGD};
    return s;
}
