#version 430 core
// One scanline of the image... 1024 is the minimum maximum
// guaranteed by OpenGL but we'll use 800
layout (local_size_x = 800) in;
// Input and output images
layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;
// Shared memory for the scanline data -- must be the same size
// as (or larger than) as the local workgroup
shared vec4 scanline[1024];
void main(void)
{
// Get the current position in the image.
ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
// Read an input pixel and store it in the shared array
scanline[pos.x] = imageLoad(input_image, pos);
// Ensure that all other invocations have reached this point
// and written their shared data by calling barrier()
barrier();
// Compute our result and write it back to the image
vec4 result = scanline[min(pos.x + 1, 799)] -
scanline[max(pos.x - 1, 0)];
imageStore(output_image, pos.yx, result);
}
// Activate the compute program...
glUseProgram(compute_prog);
// Bind the source image as input and the intermediate
// image as output
glBindImageTexture(0, input_image, 0,
GL_FALSE, 0,
GL_READ_ONLY, GL_RGBA32F);
glBindImageTexture(1, intermediate_image, 0,
GL_FALSE, 0, GL_WRITE_ONLY,
GL_RGBA32F);
// Dispatch the horizontal pass
glDispatchCompute(1, 1024, 1);
// Issue a memory barrier between the passes
glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
// Now bind the intermediate image as input and the final
// image for output
glBindImageTexture(0, intermediate_image, 0,
GL_FALSE, 0,
GL_READ_ONLY, GL_RGBA32F);
glBindImageTexture(1, output_image, 0,
GL_FALSE, 0,
GL_WRITE_ONLY, GL_RGBA32F);
// Dispatch the vertical pass
glDispatchCompute(1, 1024, 1);
