///*
// * Optimizer.h
// *
// *  Created on: 6 Dec 2017
// *      Author: Viktor
// */
//
//#ifndef OPTIMIZER_H_
//#define OPTIMIZER_H_
//
//#include <Loss.h>
//#include <NeuralNetwork.h>
//#include <Regularization.h>
//#include <vector>
//
//namespace cppnn {
//
//static const double INIT_WEIGHT_ABS_MIN = 1e-6;
//static const double INIT_WEIGHT_ABS_MAX = 1e-1;
//
//class Optimizer {
//protected:
//	const Loss& loss;
//	const Regularization& reg;
//public:
//	Optimizer(const Loss& loss, const Regularization& reg);
//	virtual ~Optimizer();
//	virtual void init_weights(NeuralNetwork& net);
//	virtual void update_weights(NeuralNetwork& net, double error) = 0;
//	virtual void train(NeuralNetwork& net,
//			std::vector<std::pair<viennacl::vector<double>,viennacl::vector<double>>>& training_data,
//			int epochs) = 0;
//};
//
//class SGDOptimizer : public Optimizer {
//public:
//	SGDOptimizer(Loss& loss, Regularization& reg);
//	virtual ~SGDOptimizer();
//	virtual void update_weights(NeuralNetwork& net, double error);
//	virtual void train(NeuralNetwork& net,
//			std::vector<std::pair<std::vector<double>,std::vector<double>>>& training_data,
//			int epochs);
//};
//
//class NadamOptimizer : public Optimizer {
//public:
//	NadamOptimizer(Loss& loss, Regularization& reg);
//	virtual ~NadamOptimizer();
//	virtual void update_weights(NeuralNetwork& net, double error);
//	virtual void train(NeuralNetwork& net,
//			std::vector<std::pair<std::vector<double>,std::vector<double>>>& training_data,
//			int epochs);
//};
//
//}
//
//#endif /* OPTIMIZER_H_ */