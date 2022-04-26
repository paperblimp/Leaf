
// header
#include "export.hpp"

// local
#include "dialogs/file_browser.hpp"
#include "graphical/graphics.hpp"
#include "utils/asserts.hpp"
#include "graphical/opengl/framebuffer.hpp"
#include "graphical/opengl/render.hpp"
#include "animation/animation.hpp"

// builtin
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <imgui.h>
#include <memory>
#include <thread>

// extern
#include <glm/vec2.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}



std::optional<std::string> browser_filter(const std::filesystem::path path)
{
    if (path.extension() != ".mp4")
        return "only \".mp4\" is supported";
    else
        return std::nullopt;
}


ExportDialog::ExportDialog(double _animation_length): animation_length(_animation_length)
{
    this->fps = 60;
    strcpy((char*)this->path.data(), (char*)std::filesystem::current_path().c_str());

    ImGui::OpenPopup("Export");
}


bool ExportDialog::run()
{
    bool close = false;
    leaf_assert(ImGui::BeginPopupModal("Export", nullptr, ImGuiWindowFlags_AlwaysAutoResize));
    

    if (this->export_process.has_value())
    {
        if (auto progress = this->export_process->get_export_progress(); progress.has_value())
        {
            ImGui::ProgressBar((float)progress.value() / 100, ImVec2(0.0f, 0.0f));
            
            if (ImGui::Button("cancel"))
            {
                this->export_process->stop();
                close = true;
            }
        }
        else
        {
            ImGui::Text("export completed");
            if (ImGui::Button("ok"))
                close = true;
        }
    }
    else
    {
        ImGui::InputText("output path", this->path.data(), this->path.size());
        ImGui::SameLine();
        if (ImGui::Button("select path"))
        {
            this->file_browser = FileBrowser{"select output path", FileBrowser::Type::File, browser_filter};
            this->file_browser->open(FileBrowser::State::default_with_current_path());
        }

        if (this->file_browser.has_value())
        {
            if (auto output = this->file_browser->run(); output.has_value())
                strcpy(this->path.data(), output.value().data());
        }

        ImGui::InputInt("fps", &this->fps);

        if (ImGui::Button("cancel"))
        {
            ImGui::CloseCurrentPopup();
            close = true;
        }

        ImGui::SameLine();
        
        if (ImGui::Button("export"))
            this->export_process = ExportProcess{this->path.data(), (uint64_t)this->fps, this->animation_length};
    }
    
    ImGui::EndPopup();
    return !close;
}



ExportProcess::ExportProcess(std::string path, uint64_t fps, double animation_length): progress_counter(std::make_shared<std::atomic_uint8_t>(0)), _stop(std::make_shared<std::atomic_bool>(false))
{
    std::thread{export_animation, path, fps, animation_length, this->progress_counter, this->_stop}.detach();
}

std::optional<uint8_t> ExportProcess::get_export_progress()
{
    auto progress = this->progress_counter->load();

    if (progress >= 100)
        return std::nullopt;

    return progress;
}

void ExportProcess::stop()
{
    this->_stop->store(true);
}




void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, FILE* output_file)
{
    int ret;

    ret = avcodec_send_frame(enc_ctx, frame);
    leaf_runtime_assert(ret >= 0);

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        leaf_runtime_assert(ret >= 0);

        fwrite(pkt->data, 1, pkt->size, output_file);
        av_packet_unref(pkt);
    }
}

void render(Framebuffer& framebuffer, double time)
{
    const auto draw_node = [&framebuffer, time](Node& node)
    {
        animate(node, time);

        auto& sprite = sprite_manager.get_sprite(node.texture_path.value());
        render_sprite(
            sprite,
            node.position,
            (glm::dvec2)sprite.size * (glm::dvec2)node.scale,
            glm::degrees(node.rotation),
            {node.rotation_pivot[0] + node.position.x, node.rotation_pivot[1] + node.position.y},
            framebuffer
        );
    };

    framebuffer.clear({255, 255, 255, 255});

    node_tree->run_on_nodes_ordered_reverse([&](Node& node)
    {
        if (node.texture_path.has_value() == false)
            return;

        if (node.visible == false)
            return;

        draw_node(node);
    });
}


void export_animation(std::string path, uint64_t fps, double length, std::shared_ptr<std::atomic_uint8_t> progress_counter, std::shared_ptr<std::atomic_bool> stop)
{
    const std::string codec_name = "mpeg2video";
    const AVCodec* codec;
    AVCodecContext* codec_context = nullptr;
    auto camera_size = (glm::u64vec2)get_camera_area();
    FILE* output_file;
    AVFrame* frame;
    AVPacket* pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

    codec = avcodec_find_encoder_by_name(codec_name.c_str());
    leaf_runtime_assert(codec != nullptr);

    codec_context = avcodec_alloc_context3(codec);
    leaf_runtime_assert(codec_context != nullptr);

    pkt = av_packet_alloc();
    leaf_runtime_assert(pkt != nullptr);
    

    codec_context->bit_rate = 400000;
    // resolution must be a multiple of two
    codec_context->width = camera_size.x + ((camera_size.x % 2 == 0) ? 0 : 1);
    codec_context->height = camera_size.y + ((camera_size.y % 2 == 0) ? 0 : 1);
    codec_context->time_base = AVRational{1, (int)fps};
    codec_context->framerate = AVRational{(int)fps, 1};
    codec_context->gop_size = 10;
    codec_context->max_b_frames = 1;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(codec_context->priv_data, "preset", "slow", 0);

    leaf_runtime_assert(avcodec_open2(codec_context, codec, NULL) >= 0);

    output_file = fopen((path + ".tmp").c_str(), "wb");
    leaf_runtime_assert(output_file != nullptr);

    frame = av_frame_alloc();
    leaf_runtime_assert(frame != nullptr);

    frame->format = codec_context->pix_fmt;
    frame->width  = codec_context->width;
    frame->height = codec_context->height;

    leaf_runtime_assert(av_frame_get_buffer(frame, 0) >= 0);

    
    
    
    graphic_context.make_current_export_context();
    auto framebuffer = Framebuffer{camera_size.x, camera_size.y};
    uint8_t* pixels = new uint8_t[camera_size.x * camera_size.y * 4];
    memset(pixels, 0, camera_size.x * camera_size.y * 4);
    SwsContext* sws_context = nullptr;

    for (uint64_t i = 0; i < (length / (1.f / fps)); ++i)
    {
        if (stop->load() == true)
        {
            delete[] pixels;
            fclose(output_file);
            avcodec_free_context(&codec_context);
            av_frame_free(&frame);
            av_packet_free(&pkt);
            return;
        }

        progress_counter->store((uint8_t)(((double)i / (double)(length / (1.f / fps))) * 100));

        render(framebuffer, (1.f / fps) * i);

        framebuffer.bind();
        glReadPixels(0, 0, camera_size.x, camera_size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        

        sws_context = sws_getCachedContext(sws_context, 
            frame->width, frame->height, AV_PIX_FMT_RGBA,
            frame->width, frame->height, AV_PIX_FMT_YUV420P,
            SWS_BICUBIC, NULL, NULL, NULL
        );

        int linesize = camera_size.x * 4;
        sws_scale(sws_context, &pixels, &linesize, 0, frame->height, frame->data, frame->linesize);

        frame->pts = i;
        leaf_runtime_assert(av_frame_make_writable(frame) >= 0);
        encode(codec_context, frame, pkt, output_file);
    }
    progress_counter->store(100);



    encode(codec_context, nullptr, pkt, output_file);

    if (codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
        fwrite(endcode, 1, sizeof(endcode), output_file);
    fclose(output_file);

    delete[] pixels;
    avcodec_free_context(&codec_context);
    av_frame_free(&frame);
    av_packet_free(&pkt);


    std::string ffmpeg_path;

    #if defined(_WIN32)
        ffmpeg_path = boost::dll::program_location().parent_path().string() + ".\\ffmpeg.exe";
    #elif defined(__linux__)
        ffmpeg_path = boost::dll::program_location().parent_path().string() + "/ffmpeg";
    #else
        #error "unsupported plataform"
    #endif

    if (std::filesystem::exists(path))
        std::filesystem::remove(path);

    auto command = fmt::format("{} -i \"{}.tmp\" -c:v copy -f mp4 \"{}\"", ffmpeg_path, path, path);
    system(command.c_str());
    std::filesystem::remove(path + ".tmp");
}
