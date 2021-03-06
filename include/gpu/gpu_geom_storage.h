#ifndef FILE_GPU_GEOM_STORAGE_H
#define FILE_GPU_GEOM_STORAGE_H

///TODO: think about a ringbuffer idea for this purpose.
/// Propably a ringbuffer is not the right thing.
/// or think about this:
/// https://www.ibm.com/developerworks/aix/tutorials/au-memorymanager/index.html
/// essentially we create four (unfortunately we can't typecast within the shader)
/// buffers: Information, Vertex, TexCoord, Triangle
/// it would be space efficient though if we put everything into the same buffer
/// if the objects have multiples of their size, we could put multiple o

#include <vector>
#include <memory>

#include <Eigen/Eigen>

#include <cuda/gpu_mesh_structure.h>
#include "gpu_buffer.h"

using namespace std;

class MeshReconstruction;
class GeometryBase;
class MeshPatch;
class MeshTexture;
struct DoubleStitch;
struct TripleStitch;
class UnsecuredTriangleCollection;
class MeshTexture;

class GpuGeomStorage;
class InformationRenderer;
class PresentationRenderer;

class MeshPatchGpuHandle;

class GpuSlottedBuffer;

class TexAtlasPatch;
class FBOConnector;
class Scheduler;

class ActiveSet;
class GeometryUpdater;

class TextureUpdater;
class LowDetailRenderer;

/**
 * @brief The GpuGeomStorage class
 * TODO: think about how data could be loaded onto and downloaded from from different threads.
 * (Propably you could easiliy add mutexes that block everything but that is not the nicest way)
 *
 * for the structures defined in scaleableMap it is as follows:
 * Most ideally we know that only the capture thread modifies data on the gpu and the remaining structure.
 * the render thread only needs to read it.
 *
 * For the GpuGeomStorage it is different:
 * We would have to use MUTEXes for the arrays we iterate over, or we use the mutex for every geometry structure!!!!!
 * It is the easiest solution and the only
 * one that secures that we do not duplicate data on the gpu.
 */
/**
 * @brief The GpuGeomStorage class
 */
class GpuGeomStorage {
	friend ActiveSet;
	friend MeshReconstruction;
	friend InformationRenderer;
	friend PresentationRenderer;
	friend Scheduler;
	friend MeshTexture;
	friend GeometryUpdater;
	
public:

	~GpuGeomStorage();

	void init(MeshReconstruction * scaleable_map) {
		map_ = scaleable_map;
	}

	void initialize();

	void resetTimers();

	shared_ptr<ActiveSet> makeActiveSet(
			vector<shared_ptr<MeshPatch> > patches,
			MeshReconstruction *map,
			LowDetailRenderer* low_detail_renderer,
			TextureUpdater* texture_updater,
			InformationRenderer* information_renderer, //TODO: no default nullptr till here
			bool initial = false, //TODO: get rid of the debug and initial parameter
			bool debug1 = false);//defaults to an empty element

	///TODO: these references to MeshPatches etc. should be shared_ptr.
	/// This would prevent them from being destroyed prematurely but still having references
	/**
	 * @brief pointers
	 * Have pointers, names and references to all the necessary gpu resources here at this place
	 * They are supposed to be connected to cuda with: cudaGraphicsGLRegisterBuffer
	 * TOTHINK: we have to protect these pointers from beeing used too early.
	 * e.g. as they are not totally uploaded yet but the pointer is set. the renderer might want to render from that.
	 * POSSIBLE SOLUTION: protect these arrays with the pointers with mutexes but add "uploadingInProgress" booleans
	 * to tell the reading threads that there is something going on.
	 * TOTHINK2: a lot of these elements could be deleted on the cpu side.
	 * having weak_ptrs in this list would definitely help managing deleting of objects.
	 * (the destructor would not have to remove pointers from this list)
	 */

	GpuBuffer<GpuVertex>       *vertex_buffer = nullptr;
	GpuBuffer<Eigen::Vector2f> *tex_pos_buffer = nullptr;
	GpuBuffer<GpuTriangle>     *triangle_buffer = nullptr;
	GpuBuffer<GpuPatchInfo>    *patch_info_buffer = nullptr;
	GpuBuffer<GLint>           *patch_info_index = nullptr;

	chrono::duration<double> time_spent_uploading_vertices;
	chrono::duration<double> time_spent_uploading_triangles;
	chrono::duration<double> time_spent_uploading_patch_infos;
	chrono::duration<double> time_spent_uploading_tex_pos;

	int upload_calls_vertices    = 0;
	int upload_calls_triangles   = 0;
	int upload_calls_patch_infos = 0;
	int upload_calls_tex_pos     = 0;

	bool debug_outputs = false;
	uint32_t debug = 1;
	//TODO: we should get this from the scaleableMap parameter structure
	//the assumption is that the average patch has 400 triangles
	uint32_t max_nr_patches = 1024 * 5 * 2  * debug;//5k patches is reasonable
	//*2 for debug because we are fragmenting too much.
	//debug because we are fragmenting too much

	uint32_t max_nr_vertices = 800 * max_nr_patches ;

	uint32_t max_nr_tex_coordinates = 1000 * max_nr_patches;

	uint32_t max_nr_triangles = 800 * max_nr_patches;
	uint32_t max_nrtriangles_in_collection = 800;

	uint32_t max_nr_loaded_patch_infos = max_nr_patches;//TODO: implement this

private:

	//TODO: add another buffer with references from texture points to vertices!!!!!!

	//TODO: get rid of these and spearate the reservation of slots from
	//the upload of slots

	shared_ptr<TexCoordBufConnector> uploadMeshTexCoords_(shared_ptr<MeshTexture> coords);
	shared_ptr<TriangleBufConnector> uploadTriangles_(GeometryBase *base_element);
	void uploadMeshPatch_(MeshPatch *patch, ActiveSet *active_set);
	void unloadMeshPatch_(MeshPatch *patch);
	void unloadDoubleStitch_(DoubleStitch *stitch);
	/**
	 * @brief uploadTripleStitch
	 * again triple stitches, they should be collected and put into one buffer, but only when needed.
	 */
	void uploadTripleStitches_(vector<TripleStitch*> triple_stitches);

	uint64_t delete_debug_tex_reference_;
	MeshReconstruction *map_;

};

#endif // FILE_GPU_GEOM_STORAGE_H
