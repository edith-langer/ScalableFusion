#ifndef FILE_SCHEDULER_H
#define FILE_SCHEDULER_H

#include <thread>
#include <mutex>

#include <Eigen/Eigen>

using namespace std;
using namespace Eigen;

class MeshReconstruction;
namespace video {
	class Source;
}
struct GLFWwindow;
class ActiveSet;

class Worker;
class GpuStorage;


class Segmentation;

class GarbageCollector;

class LowDetailRenderer;

namespace gfx {
	class GpuTex2D;
} // namespace gfx

//TODO: create a base class for the scheduler
class SchedulerBase {
public:

	virtual ~SchedulerBase() { }

	static GLFWwindow *createConnectedGlContext(GLFWwindow *context);
	static void initializeGlContextInThread(GLFWwindow *context);


	virtual void pause(bool pause) = 0;
	virtual void nextStep() = 0;

	virtual Matrix4f getLastKnownDepthPose() = 0;



	//this design allows for only one render cam but if there will be more needed we cross that bridge another time
	virtual void setRenderCamPose(Matrix4f camPose) = 0;
	virtual shared_ptr<ActiveSet> getActiveSetRendering() = 0;

	//we probably also want to update the low detail renderer whenever we update the render cam pose
	//with update i mean recreate and replace
	virtual shared_ptr<LowDetailRenderer> getLowDetailRenderer() = 0;

};

class SchedulerLinear : public SchedulerBase {
public:

	SchedulerLinear(shared_ptr<MeshReconstruction> map,
					GpuStorage* gpu_storage, video::Source *source,
	                GLFWwindow *context,
	                LowDetailRenderer *low_detail_renderer);

	~SchedulerLinear();

	//TODO: implement pause and step trough functionality
	void pause(bool pause) {
		paused_ = pause;
	}
	void nextStep() {
		take_next_step_ = true;
	}

	Matrix4f getLastKnownDepthPose() {
		return last_known_depth_pose_;
	}

	virtual void setRenderCamPose(Matrix4f camPose){

	}
	virtual shared_ptr<ActiveSet> getActiveSetRendering(){
		return nullptr;
	}

	//we probably also want to update the low detail renderer whenever we update the render cam pose
	//with update i mean recreate and replace
	virtual shared_ptr<LowDetailRenderer> getLowDetailRenderer(){
		return nullptr;
	}
private:
	//the active sets used for several operations like rendering and update
	mutex active_sets_mutex;
	vector<shared_ptr<ActiveSet>> active_sets;


	void captureWorker_(shared_ptr<MeshReconstruction> reconstruction, 
	                    video::Source *source, GLFWwindow *context);

	thread capture_thread_;

	int expand_interval_;

	Matrix4f last_known_depth_pose_;

	bool end_threads_;
	GpuStorage *gpu_storage_;



	LowDetailRenderer* low_detail_renderer_;
	bool paused_;
	bool take_next_step_;
};

#endif // FILE_SCHEDULER_H
