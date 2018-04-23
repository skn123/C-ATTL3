/*
 * gradient_test.hpp
 *
 *  Created on: 19.04.2018
 *      Author: Viktor Csomor
 */

#ifndef GRADIENT_TEST_HPP_
#define GRADIENT_TEST_HPP_

#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cattle/Cattle.hpp"

namespace cattle {

/**
 * A namespace for C-ATTL3's test functions.
 */
namespace test {

using namespace internal;

/**
 * A trait struct for the name of a scalar type and the default numeric constants used for gradient
 * verification depending on the scalar type.
 */
template<typename Scalar>
struct ScalarTraits {
	static constexpr Scalar step_size = 1e-4;
	static constexpr Scalar abs_epsilon = NumericUtils<Scalar>::EPSILON2;
	static constexpr Scalar rel_epsilon = NumericUtils<Scalar>::EPSILON3;
	inline static std::string name() {
		return "double";
	}
};

/**
 * Template specialization for single precision floating point scalars.
 */
template<>
struct ScalarTraits<float> {
	static constexpr float step_size = 5e-4;
	static constexpr float abs_epsilon = 5e-2;
	static constexpr float rel_epsilon = 5e-2;
	inline static std::string name() {
		return "float";
	}
};

/**
 * @param name The name of the gradient test.
 * @param prov The data provider to use for gradient checking.
 * @param net The neural network whose differentiation is to be checked.
 * @param opt The optimizer to use for the gradient check.
 * @param step_size The step size for numerical differentiation.
 * @param abs_epsilon The maximum acceptable absolute difference between the analytic and
 * numerical gradients.
 * @param rel_epsilon The maximum acceptable relative difference between the analytic and
 * numerical gradients.
 */
template<typename Scalar, std::size_t Rank, bool Sequential>
inline void grad_test(std::string name, DataProvider<Scalar,Rank,Sequential>& prov,
		NeuralNetwork<Scalar,Rank,Sequential>& net, Optimizer<Scalar,Rank,Sequential>& opt,
		Scalar step_size, Scalar abs_epsilon, Scalar rel_epsilon) {
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	std::string header = "*   GRADIENT CHECK: " + name + "; SCALAR TYPE: " +
			ScalarTraits<Scalar>::name() + "; RANK: " + std::to_string(Rank) + "   *";
	std::size_t header_length = header.length();
	std::string header_border = std::string(header_length, '*');
	std::string header_padding = "*" + std::string(header_length - 2, ' ') + "*";
	std::cout << std::endl << header_border << std::endl << header_padding << std::endl <<
			header << std::endl << header_padding << std::endl << header_border << std::endl;
	EXPECT_TRUE(opt.verify_gradients(net, prov, step_size, abs_epsilon, rel_epsilon));
}

/**
 * @param dims The dimensions of the random tensor to create.
 * @return A tensor of the specified dimensions filled with random values in the range of
 * -1 to 1.
 */
template<typename Scalar, std::size_t Rank>
inline TensorPtr<Scalar,Rank> random_tensor(const std::array<std::size_t,Rank>& dims) {
	TensorPtr<Scalar,Rank> tensor_ptr(new Tensor<Scalar,Rank>(dims));
	tensor_ptr->setRandom();
	return tensor_ptr;
}

/************************
 * LAYER GRADIENT TESTS *
 ************************/

/**
 * @param name The name of the gradient test.
 * @param layer1 The first instance of the layer class to verify.
 * @param layer2 The second instance.
 * @param samples The number of samples to use.
 * @param step_size The step size for numerical differentiation.
 * @param abs_epsilon The maximum acceptable absolute difference between the analytic and
 * numerical gradients.
 * @param rel_epsilon The maximum acceptable relative difference between the analytic and
 * numerical gradients.
 */
template<typename Scalar, std::size_t Rank>
inline void layer_grad_test(std::string name, LayerPtr<Scalar,Rank> layer1, LayerPtr<Scalar,Rank> layer2,
		std::size_t samples = 5, Scalar step_size = ScalarTraits<Scalar>::step_size,
		Scalar abs_epsilon = ScalarTraits<Scalar>::abs_epsilon,
		Scalar rel_epsilon = ScalarTraits<Scalar>::rel_epsilon) {
	assert(layer1->get_output_dims() == layer2->get_input_dims());
	std::array<std::size_t,Rank + 1> input_dims = layer1->get_input_dims().template promote<>();
	std::array<std::size_t,Rank + 1> output_dims = layer2->get_output_dims().template promote<>();
	TensorPtr<Scalar,Rank + 1> obs = random_tensor<Scalar,Rank + 1>(input_dims);
	TensorPtr<Scalar,Rank + 1> obj = random_tensor<Scalar,Rank + 1>(output_dims);
	MemoryDataProvider<Scalar,Rank,false> prov(std::move(obs), std::move(obj));
	std::vector<LayerPtr<Scalar,Rank>> layers(2);
	layers[0] = std::move(layer1);
	layers[1] = std::move(layer2);
	FeedforwardNeuralNetwork<Scalar,Rank> nn(std::move(layers));
	nn.init();
	auto loss = LossSharedPtr<Scalar,Rank,false>(new QuadraticLoss<Scalar,Rank,false>());
	VanillaSGDOptimizer<Scalar,Rank,false> opt(loss, samples);
	grad_test<Scalar,Rank,false>(name, prov, nn, opt, step_size, abs_epsilon, rel_epsilon);
}

/**
 * Performs gradient checks on fully-connected layers.
 */
template<typename Scalar>
inline void fc_layer_grad_test() {
	auto init1 = WeightInitSharedPtr<Scalar>(new GlorotWeightInitialization<Scalar>());
	auto init2 = WeightInitSharedPtr<Scalar>(new LeCunWeightInitialization<Scalar>());
	auto reg1 = ParamRegSharedPtr<Scalar>(new L1ParameterRegularization<Scalar>());
	auto reg2 = ParamRegSharedPtr<Scalar>(new L2ParameterRegularization<Scalar>());
	auto reg3 = ParamRegSharedPtr<Scalar>(new ElasticNetParameterRegularization<Scalar>());
	LayerPtr<Scalar,1> layer1_1(new FCLayer<Scalar,1>(Dimensions<std::size_t,1>({ 32 }), 16, init1));
	LayerPtr<Scalar,1> layer1_2(new FCLayer<Scalar,1>(layer1_1->get_output_dims(), 1, init1, reg3));
	layer_grad_test<Scalar,1>("fc layer", std::move(layer1_1), std::move(layer1_2), 5, ScalarTraits<Scalar>::step_size,
			ScalarTraits<float>::abs_epsilon, ScalarTraits<float>::rel_epsilon);
	LayerPtr<Scalar,2> layer2_1(new FCLayer<Scalar,2>(Dimensions<std::size_t,2>({ 6, 6 }), 16, init2));
	LayerPtr<Scalar,2> layer2_2(new FCLayer<Scalar,2>(layer2_1->get_output_dims(), 2, init2, reg1));
	layer_grad_test<Scalar,2>("fc layer", std::move(layer2_1), std::move(layer2_2), 5, ScalarTraits<Scalar>::step_size,
			ScalarTraits<float>::abs_epsilon, ScalarTraits<float>::rel_epsilon);
	LayerPtr<Scalar,3> layer3_1(new FCLayer<Scalar,3>(Dimensions<std::size_t,3>({ 4, 4, 2 }), 16, init1, reg2));
	LayerPtr<Scalar,3> layer3_2(new FCLayer<Scalar,3>(layer3_1->get_output_dims(), 4, init1, reg2));
	layer_grad_test<Scalar,3>("fc layer", std::move(layer3_1), std::move(layer3_2));
}

TEST(gradient_test, fc_layer_gradient_test) {
	fc_layer_grad_test<float>();
	fc_layer_grad_test<double>();
}

/**
 * Performs gradient checks on convolutional layers.
 */
template<typename Scalar>
inline void conv_layer_grad_test() {
	auto init = WeightInitSharedPtr<Scalar>(new HeWeightInitialization<Scalar>());
	auto reg = ParamRegSharedPtr<Scalar>(new L2ParameterRegularization<Scalar>());
	LayerPtr<Scalar,3> layer1(new ConvLayer<Scalar>(Dimensions<std::size_t,3>({ 8, 8, 2 }), 5, init,
			ConvLayer<Scalar>::NO_PARAM_REG, 3, 2, 1, 2, 1, 2, 1, 0));
	LayerPtr<Scalar,3> layer2(new ConvLayer<Scalar>(layer1->get_output_dims(), 1, init, reg, 1, 1));
	layer_grad_test<Scalar,3>("convolutional layer ", std::move(layer1), std::move(layer2));
}

TEST(gradient_test, conv_layer_gradient_test) {
	conv_layer_grad_test<float>();
	conv_layer_grad_test<double>();
}

/**
 * @param dims The dimensions of the layers' input tensors.
 */
template<typename Scalar, std::size_t Rank>
inline void activation_layer_grad_test(const Dimensions<std::size_t,Rank>& dims) {
	auto init = WeightInitSharedPtr<Scalar>(new GlorotWeightInitialization<Scalar>());
	LayerPtr<Scalar,Rank> layer1_1(new FCLayer<Scalar,Rank>(dims, 12, init));
	Dimensions<std::size_t,Rank> dims_1 = layer1_1->get_output_dims();
	layer_grad_test<Scalar,Rank>("identity activation layer", std::move(layer1_1),
			LayerPtr<Scalar,Rank>(new IdentityActivationLayer<Scalar,Rank>(dims_1)));
	LayerPtr<Scalar,Rank> layer1_2(new FCLayer<Scalar,Rank>(dims, 8, init));
	Dimensions<std::size_t,Rank> dims_2 = layer1_2->get_output_dims();
	layer_grad_test<Scalar,Rank>("scaling activation layer", std::move(layer1_2),
			LayerPtr<Scalar,Rank>(new ScalingActivationLayer<Scalar,Rank>(dims_2, 1.5)));
	LayerPtr<Scalar,Rank> layer1_3(new FCLayer<Scalar,Rank>(dims, 12, init));
	Dimensions<std::size_t,Rank> dims_3 = layer1_3->get_output_dims();
	layer_grad_test<Scalar,Rank>("binary step activation layer", std::move(layer1_3),
			LayerPtr<Scalar,Rank>(new BinaryStepActivationLayer<Scalar,Rank>(dims_3)));
	LayerPtr<Scalar,Rank> layer1_4(new FCLayer<Scalar,Rank>(dims, 16, init));
	Dimensions<std::size_t,Rank> dims_4 = layer1_4->get_output_dims();
	layer_grad_test<Scalar,Rank>("sigmoid activation layer rank", std::move(layer1_4),
			LayerPtr<Scalar,Rank>(new SigmoidActivationLayer<Scalar,Rank>(dims_4)));
	LayerPtr<Scalar,Rank> layer1_5(new FCLayer<Scalar,Rank>(dims, 12, init));
	Dimensions<std::size_t,Rank> dims_5 = layer1_5->get_output_dims();
	layer_grad_test<Scalar,Rank>("tanh activation layer rank", std::move(layer1_5),
			LayerPtr<Scalar,Rank>(new TanhActivationLayer<Scalar,Rank>(dims_5)));
	LayerPtr<Scalar,Rank> layer1_6(new FCLayer<Scalar,Rank>(dims, 9, init));
	Dimensions<std::size_t,Rank> dims_6 = layer1_6->get_output_dims();
	layer_grad_test<Scalar,Rank>("softplus activation layer rank", std::move(layer1_6),
			LayerPtr<Scalar,Rank>(new SoftplusActivationLayer<Scalar,Rank>(dims_6)));
	LayerPtr<Scalar,Rank> layer1_7(new FCLayer<Scalar,Rank>(dims, 12, init));
	Dimensions<std::size_t,Rank> dims_7 = layer1_7->get_output_dims();
	layer_grad_test<Scalar,Rank>("softmax activation layer rank", std::move(layer1_7),
			LayerPtr<Scalar,Rank>(new SoftmaxActivationLayer<Scalar,Rank>(dims_7)));
	LayerPtr<Scalar,Rank> layer1_8(new FCLayer<Scalar,Rank>(dims, 10, init));
	Dimensions<std::size_t,Rank> dims_8 = layer1_8->get_output_dims();
	layer_grad_test<Scalar,Rank>("relu activation layer rank", std::move(layer1_8),
			LayerPtr<Scalar,Rank>(new ReLUActivationLayer<Scalar,Rank>(dims_8)));
	LayerPtr<Scalar,Rank> layer1_9(new FCLayer<Scalar,Rank>(dims, 12, init));
	Dimensions<std::size_t,Rank> dims_9 = layer1_9->get_output_dims();
	layer_grad_test<Scalar,Rank>("leaky relu activation layer rank", std::move(layer1_9),
			LayerPtr<Scalar,Rank>(new LeakyReLUActivationLayer<Scalar,Rank>(dims_9, 2e-1)));
	LayerPtr<Scalar,Rank> layer1_10(new FCLayer<Scalar,Rank>(dims, 11, init));
	Dimensions<std::size_t,Rank> dims_10 = layer1_10->get_output_dims();
	layer_grad_test<Scalar,Rank>("elu activation layer rank", std::move(layer1_10),
			LayerPtr<Scalar,Rank>(new ELUActivationLayer<Scalar,Rank>(dims_10, 2e-1)));
	auto reg = ParamRegSharedPtr<Scalar>(new L2ParameterRegularization<Scalar>());
	layer_grad_test<Scalar,Rank>("prelu activation layer rank",
			LayerPtr<Scalar,Rank>(new PReLUActivationLayer<Scalar,Rank>(dims, reg, 2e-1)),
			LayerPtr<Scalar,Rank>(new PReLUActivationLayer<Scalar,Rank>(dims)));
}

/**
 * Template specialization.
 *
 * @param dims The dimensions of the layers' input tensors.
 */
template<typename Scalar>
inline void activation_layer_grad_test(const Dimensions<std::size_t,3>& dims) {
	auto init = WeightInitSharedPtr<Scalar>(new HeWeightInitialization<Scalar>());
	LayerPtr<Scalar,3> layer1_1(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_1 = layer1_1->get_output_dims();
	layer_grad_test<Scalar,3>("identity activation layer", std::move(layer1_1),
			LayerPtr<Scalar,3>(new IdentityActivationLayer<Scalar,3>(dims_1)));
	LayerPtr<Scalar,3> layer1_2(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_2 = layer1_2->get_output_dims();
	layer_grad_test<Scalar,3>("scaling activation layer", std::move(layer1_2),
			LayerPtr<Scalar,3>(new ScalingActivationLayer<Scalar,3>(dims_2, 1.5)));
	LayerPtr<Scalar,3> layer1_3(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_3 = layer1_3->get_output_dims();
	layer_grad_test<Scalar,3>("binary step activation layer", std::move(layer1_3),
			LayerPtr<Scalar,3>(new BinaryStepActivationLayer<Scalar,3>(dims_3)));
	LayerPtr<Scalar,3> layer1_4(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_4 = layer1_4->get_output_dims();
	layer_grad_test<Scalar,3>("sigmoid activation layer", std::move(layer1_4),
			LayerPtr<Scalar,3>(new SigmoidActivationLayer<Scalar,3>(dims_4)));
	LayerPtr<Scalar,3> layer1_5(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_5 = layer1_5->get_output_dims();
	layer_grad_test<Scalar,3>("tanh activation layer", std::move(layer1_5),
			LayerPtr<Scalar,3>(new TanhActivationLayer<Scalar,3>(dims_5)));
	LayerPtr<Scalar,3> layer1_6(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_6 = layer1_6->get_output_dims();
	layer_grad_test<Scalar,3>("softplus activation layer", std::move(layer1_6),
			LayerPtr<Scalar,3>(new SoftplusActivationLayer<Scalar,3>(dims_6)));
	LayerPtr<Scalar,3> layer1_7(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_7 = layer1_7->get_output_dims();
	layer_grad_test<Scalar,3>("softmax activation layer", std::move(layer1_7),
			LayerPtr<Scalar,3>(new SoftmaxActivationLayer<Scalar,3>(dims_7)));
	LayerPtr<Scalar,3> layer1_8(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_8 = layer1_8->get_output_dims();
	layer_grad_test<Scalar,3>("relu activation layer", std::move(layer1_8),
			LayerPtr<Scalar,3>(new ReLUActivationLayer<Scalar,3>(dims_8)));
	LayerPtr<Scalar,3> layer1_9(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_9 = layer1_9->get_output_dims();
	layer_grad_test<Scalar,3>("leaky relu activation layer", std::move(layer1_9),
			LayerPtr<Scalar,3>(new LeakyReLUActivationLayer<Scalar,3>(dims_9, 2e-1)));
	LayerPtr<Scalar,3> layer1_10(new ConvLayer<Scalar>(dims, 2, init));
	Dimensions<std::size_t,3> dims_10 = layer1_10->get_output_dims();
	layer_grad_test<Scalar,3>("elu activation layer", std::move(layer1_10),
			LayerPtr<Scalar,3>(new ELUActivationLayer<Scalar,3>(dims_10, 2e-1)));
	auto reg = ParamRegSharedPtr<Scalar>(new L2ParameterRegularization<Scalar>());
	layer_grad_test<Scalar,3>("prelu activation layer",
			LayerPtr<Scalar,3>(new PReLUActivationLayer<Scalar,3>(dims, reg, 2e-1)),
			LayerPtr<Scalar,3>(new PReLUActivationLayer<Scalar,3>(dims)));
}

/**
 * Performs gradient checks on all activation layers.
 */
template<typename Scalar>
inline void activation_layer_grad_test() {
	Dimensions<std::size_t,1>({ 24 });
	activation_layer_grad_test<Scalar,1>(Dimensions<std::size_t,1>({ 24 }));
	activation_layer_grad_test<Scalar,2>(Dimensions<std::size_t,2>({ 5, 5 }));
	activation_layer_grad_test<Scalar,3>(Dimensions<std::size_t,3>({ 4, 3, 2 }));
}

TEST(gradient_test, activation_layer_gradient_test) {
	activation_layer_grad_test<float>();
	activation_layer_grad_test<double>();
}

/**
 * Performs gradient checks on all pooling layers.
 */
template<typename Scalar>
inline void pooling_layer_grad_test() {
	Dimensions<std::size_t,3> dims({ 16, 16, 2 });
	auto init = WeightInitSharedPtr<Scalar>(new HeWeightInitialization<Scalar>());
	LayerPtr<Scalar,3> sum_layer1(new ConvLayer<Scalar>(dims, 2, init));
	LayerPtr<Scalar,3> sum_layer2(new SumPoolingLayer<Scalar>(sum_layer1->get_output_dims(), 3, 1, 1, 2, 0, 1));
	layer_grad_test<Scalar,3>("sum pooling layer", std::move(sum_layer1), std::move(sum_layer2));
	LayerPtr<Scalar,3> mean_layer1(new ConvLayer<Scalar>(dims, 2, init));
	LayerPtr<Scalar,3> mean_layer2(new MeanPoolingLayer<Scalar>(mean_layer1->get_output_dims(), 3, 1, 1, 2, 0, 1));
	layer_grad_test<Scalar,3>("mean pooling layer", std::move(mean_layer1), std::move(mean_layer2));
	LayerPtr<Scalar,3> max_layer1(new ConvLayer<Scalar>(dims, 2, init));
	LayerPtr<Scalar,3> max_layer2(new MaxPoolingLayer<Scalar>(max_layer1->get_output_dims(), 3, 1, 1, 2, 0, 1));
	layer_grad_test<Scalar,3>("max pooling layer", std::move(max_layer1), std::move(max_layer2));
}

TEST(gradient_test, pooling_layer_gradient_test) {
	pooling_layer_grad_test<float>();
	pooling_layer_grad_test<double>();
}

/**
 * Performs gradient checks on batch normalization layers.
 */
template<typename Scalar>
inline void batch_norm_layer_grad_test() {
	auto init = WeightInitSharedPtr<Scalar>(new HeWeightInitialization<Scalar>());
	auto reg1 = ParamRegSharedPtr<Scalar>(new L1ParameterRegularization<Scalar>());
	auto reg2 = ParamRegSharedPtr<Scalar>(new L2ParameterRegularization<Scalar>());
	LayerPtr<Scalar,1> layer1_1(new FCLayer<Scalar,1>(Dimensions<std::size_t,1>({ 32 }), 16, init));
	LayerPtr<Scalar,1> layer1_2(new BatchNormLayer<Scalar,1>(layer1_1->get_output_dims(), reg2, reg1));
	layer_grad_test<Scalar,1>("batch norm layer", std::move(layer1_1), std::move(layer1_2), 5,
			ScalarTraits<Scalar>::step_size, ScalarTraits<float>::abs_epsilon, ScalarTraits<float>::rel_epsilon);
	LayerPtr<Scalar,2> layer2_1(new BatchNormLayer<Scalar,2>(Dimensions<std::size_t,2>({ 6, 6 })));
	LayerPtr<Scalar,2> layer2_2(new IdentityActivationLayer<Scalar,2>(layer2_1->get_output_dims()));
	layer_grad_test<Scalar,2>("batch norm layer", std::move(layer2_1), std::move(layer2_2));
	LayerPtr<Scalar,3> layer3_1(new ConvLayer<Scalar>(Dimensions<std::size_t,3>({ 4, 4, 2 }), 2, init));
	LayerPtr<Scalar,3> layer3_2(new BatchNormLayer<Scalar,3>(layer3_1->get_output_dims(), reg2, reg2));
	layer_grad_test<Scalar,3>("batch norm layer", std::move(layer3_1), std::move(layer3_2));
}

TEST(gradient_test, batch_norm_layer_grad_test) {
	batch_norm_layer_grad_test<float>();
	batch_norm_layer_grad_test<double>();
}

/*********************************
 * NEURAL NETWORK GRADIENT TESTS *
 *********************************/

} /* namespace test */

} /* namespace cattle */

#endif /* GRADIENT_TEST_HPP_ */