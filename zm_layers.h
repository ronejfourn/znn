#pragma once

#include "zm_tensor.h"

struct zm_layer;

typedef void(*zm_layer_forward_fxn)(struct zm_layer *, const zm_tensor *);
typedef void(*zm_layer_backward_fxn)();

typedef struct zm_layer {
    const zm_tensor *input;
    zm_tensor output;
    zm_layer_forward_fxn forward;
    zm_layer_backward_fxn backward;
    void *layer_data;
} zm_layer;

typedef struct zm_sequential {
    zm_layer *layers;
    u32 n_layers;
} zm_sequential;

zm_layer zm_layer_linear(u32 in_features, u32 out_features);
zm_layer zm_layer_flatten();
zm_layer zm_layer_ReLU();
zm_layer zm_layer_softmax(u32 dim);

zm_sequential zm_sequential_create(u32 n_layers, zm_layer *layers);
const zm_tensor zm_sequential_forward(zm_sequential *s, const zm_tensor *input);