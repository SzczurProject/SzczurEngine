#include "Szczur/Modules/Cinematics/MoviePlayer.hpp"

bool MoviePlayer::loadFromFile(const char * filename)
{
    av_register_all();

    if(avformat_open_input(&m_pFormatCtx, filename, NULL, NULL)!=0)
        return false; 

    if(avformat_find_stream_info(m_pFormatCtx, NULL)<0)
        return false; 
    
    av_dump_format(m_pFormatCtx, 0, filename, 0);
    
    m_videoStream = -1;
    m_audioStream = -1;
    for(int i = 0; i < m_pFormatCtx->nb_streams; ++i)
    {
        if(m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_videoStream = i;
        }
        else if(m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            m_audioStream = i;
        }
    }
    
    if(m_videoStream < 0)
        return false;

    if(m_videoStream >= 0)
    {
        m_pCodecCtx = m_pFormatCtx->streams[m_videoStream]->codec;
        m_pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
        
        if(avcodec_open2(m_pCodecCtx, m_pCodec, &m_optionsDict)<0)
            return false;
    }
    if(m_audioStream >= 0)
    {
        m_paCodecCtx = m_pFormatCtx->streams[m_audioStream]->codec;
        m_paCodec = avcodec_find_decoder(m_paCodecCtx->codec_id);
        
        if(avcodec_open2(m_paCodecCtx, m_paCodec, &m_optionsDictA))
            return false;
    }

    m_pFrame = av_frame_alloc();
    m_pFrameRGB = av_frame_alloc();
    if (m_pFrameRGB == nullptr)
    {
        return false;
    }

    m_numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, m_pCodecCtx->width, m_pCodecCtx->height);
    m_buffer = (uint8_t*)av_malloc(m_numBytes * sizeof(uint8_t));
    
    m_sws_ctx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt, m_pCodecCtx->width, m_pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    
    avpicture_fill((AVPicture*)m_pFrameRGB, m_buffer, AV_PIX_FMT_RGB24, m_pCodecCtx->width, m_pCodecCtx->height);
    return true;
}

void MoviePlayer::jumpTo(const unsigned int &seekTarget)
{
    if(m_sound)
    {
        for(auto p : m_sound->g_videoPkts)
        {
            av_free_packet(p);
           // av_free(p);
        }
        m_sound->g_videoPkts.clear();
        double fps = av_q2d(m_pFormatCtx->streams[m_videoStream]->r_frame_rate);
        int64_t seekOnVideo = seekTarget * (fps/1000000);

        m_ICurrentFrame = seekOnVideo;

        seekOnVideo = seekOnVideo *
             (m_pFormatCtx->streams[m_videoStream]->time_base.den /
              m_pFormatCtx->streams[m_videoStream]->time_base.num) /
             (m_pFormatCtx->streams[m_videoStream]->codec->time_base.den /
              m_pFormatCtx->streams[m_videoStream]->codec->time_base.num )*
              m_pCodecCtx->ticks_per_frame;
        auto ret = avformat_seek_file(m_pFormatCtx, m_videoStream, 0, seekOnVideo, seekOnVideo, AVSEEK_FLAG_FRAME);
        assert(ret >= 0);
        avcodec_flush_buffers(m_pCodecCtx);
        m_syncAV = true;
    }
}

void MoviePlayer::play()
{
    auto &window = getModule<rat::Window>().getWindow(); 
    for(auto p : m_loops)
    {
        p->setFont(m_font);
        p->init(window.getSize());
    }

    int64_t duration = m_pFormatCtx->duration;
    const int FrameSize = m_pCodecCtx->width * m_pCodecCtx->height * 3;

     sf::Uint8* Data = new sf::Uint8[m_pCodecCtx->width * m_pCodecCtx->height * 4];
    

    m_im_video.create(m_pCodecCtx->width, m_pCodecCtx->height);
    m_im_video.setSmooth(false);

    sf::Sprite sprite(m_im_video);

    float x = window.getSize().x;
    float y = window.getSize().y;
    sprite.setScale(x/m_im_video.getSize().x,y/m_im_video.getSize().y);

    m_sound = new MovieSound(m_pFormatCtx,m_audioStream);
    m_sound->play();
    m_VClock = new sf::Clock;
    int IdeltaTime;
    int IstartTime;

    size_t count =  m_loops.size();
    
    float IframeTime = 1000.f/av_q2d(m_pFormatCtx->streams[m_videoStream]->r_frame_rate);
 
    while (window.isOpen())
    {
        sf::sleep(sf::milliseconds(1));
        if((max>m_sound->timeElapsed()*1000 && !m_syncAV&& max>duration-4000000)||(m_sound->g_audioPkts.empty()&&max>duration-4000000))
        {
            while(window.isOpen()&&!m_sound->g_videoPkts.empty())
            {
                IstartTime = m_VClock->getElapsedTime().asMilliseconds();
                AVPacket *packet_ptr = m_sound->g_videoPkts.front();

                auto decodedLength = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &m_frameFinished, packet_ptr);
    
                if(!sws_scale(m_sws_ctx, (uint8_t const * const *)m_pFrame->data, m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrameRGB->data, m_pFrameRGB->linesize))
                {
                    break;
                }
                            
                for (int i = 0, j = 0; i < FrameSize; i += 3, j += 4)
                {
                    Data[j + 0] = m_pFrameRGB->data[0][i + 0];
                    Data[j + 1] = m_pFrameRGB->data[0][i + 1];
                    Data[j + 2] = m_pFrameRGB->data[0][i + 2];
                    Data[j + 3] = 255;
                }
                
                m_im_video.update(Data);
                
                window.clear();
                    
                window.draw(sprite);
                m_sound->g_videoPkts.erase(m_sound->g_videoPkts.begin());
                if(!m_loops.empty() && m_loops[ICurrentLoop]) m_loops[ICurrentLoop]->draw();
                window.display();
                IdeltaTime = m_VClock->getElapsedTime().asMilliseconds() - IstartTime;
                sf::sleep(sf::milliseconds((1000.f/av_q2d(m_pFormatCtx->streams[m_videoStream]->avg_frame_rate))-IdeltaTime));
            }
            for(auto p : m_sound->g_videoPkts)
            {
                av_free_packet(p);
               // av_free(p);
            }
            for(auto p : m_sound->g_audioPkts)
            {
                av_free_packet(p);
                av_free(p);
            }
            sws_freeContext(m_sws_ctx);
            av_free(m_buffer);
            av_free(m_pFrameRGB);
            av_free(m_pFrame);
            avcodec_close(m_pCodecCtx);
            avcodec_close(m_paCodecCtx);
            avformat_close_input(&m_pFormatCtx);
            window.clear();
            window.display();
            delete [] Data;
            return;
        }

        if(max<m_sound->timeElapsed()*1000)
        {
            max = m_sound->timeElapsed()*1000;
        }
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
            {
                window.close();
            }
            if(!m_loops.empty()) 
            {
                if(m_loops[ICurrentLoop] && event.type == sf::Event::KeyReleased && (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down))
                {
                    m_loops[ICurrentLoop]->change();
                }
                if( m_loops[ICurrentLoop] && m_loops[ICurrentLoop]->getStartTime()<= max &&event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return)
                {
                    m_loops[ICurrentLoop]->setDraw(false);
                    m_loops[ICurrentLoop] = nullptr;
                    ICurrentLoop++;
                    if(ICurrentLoop==count)
                    {
                        ICurrentLoop--;
                    }
                    if(m_loops[ICurrentLoop]) m_loops[ICurrentLoop]->setTime(max);
                }
            }
        }
        AVPacket* packet_ptr = nullptr;
        
        if(m_sound->g_videoPkts.size() < 150)
        {
            packet_ptr = (AVPacket*)av_malloc(sizeof(AVPacket));
            av_init_packet(packet_ptr);
            
            if(av_read_frame(m_pFormatCtx, packet_ptr) < 0)
            {
                if(m_sound->g_videoPkts.empty() || m_sound->g_audioPkts.empty())
                {
                    av_free_packet(packet_ptr);
                    av_free(packet_ptr);
                    packet_ptr = nullptr;
                }
                av_free_packet(packet_ptr);
                av_free(packet_ptr);
            }
            if(packet_ptr)
            {
                AVPacket& packet = *packet_ptr;
                if(packet.stream_index == m_videoStream)
                {
                    m_sound->g_videoPkts.push_back(packet_ptr);
                }
                else if(packet.stream_index == m_audioStream)
                {
                    if(packet_ptr->pts >= m_blockPts && !m_syncAV)
                    {
                         std::lock_guard<std::mutex> lk(m_sound->g_mut);
                        for(auto p : m_audioSyncBuffer)
                        {
                            if(p->pts >= m_blockPts)
                            {
                                m_sound->g_audioPkts.push_back(p);
                            }
                            else
                            {
                                av_free_packet(p);
                                av_free(p);
                            }
                        }
                    
                        m_sound->g_audioPkts.push_back(packet_ptr);
                        m_sound->g_newPktCondition.notify_one();
                           
                        m_audioSyncBuffer.clear();
                    }
                    
                    if(m_syncAV)
                    {
                        m_audioSyncBuffer.push_back(packet_ptr);
                    }
                    
                }
            }
   
        }

        const auto pStream = m_pFormatCtx->streams[m_videoStream];
        if(m_sound->timeElapsed() > IframeTime * m_ICurrentFrame && m_sound->isAudioReady() && !m_sound->g_videoPkts.empty())
        {
            packet_ptr = m_sound->g_videoPkts.front();
            m_sound->g_videoPkts.pop_front();
            
            auto decodedLength = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &m_frameFinished, packet_ptr);
            
            if(m_frameFinished)
            {
                m_ICurrentFrame++;
                sws_scale(m_sws_ctx, (uint8_t const * const *)m_pFrame->data, m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrameRGB->data, m_pFrameRGB->linesize);
                
                for (int i = 0, j = 0; i < FrameSize; i += 3, j += 4)
                {
                    Data[j + 0] = m_pFrameRGB->data[0][i + 0];
                    Data[j + 1] = m_pFrameRGB->data[0][i + 1];
                    Data[j + 2] = m_pFrameRGB->data[0][i + 2];
                    Data[j + 3] = 255;
                }
                m_im_video.update(Data);
                
                window.clear();
                
                window.draw(sprite);
                
                if(!m_loops.empty() && m_loops[ICurrentLoop]) m_loops[ICurrentLoop]->draw();
                window.display();
                           
                if(m_syncAV)
                {
                    m_blockPts = IframeTime * m_ICurrentFrame;
                    m_sound->setPlayingOffset(sf::milliseconds(IframeTime * m_ICurrentFrame));
                    max = m_sound->timeElapsed()*1000;
                    if(!m_loops.empty()) m_loops[ICurrentLoop]->setTime(max);
                    m_syncAV = false;
                }
                
            }
           
            if(decodedLength < packet_ptr->size)
            {
                packet_ptr->data += decodedLength;
                packet_ptr->size -= decodedLength;
                
                m_sound->g_videoPkts.push_front(packet_ptr);
            }
            else
            {
                av_free_packet(packet_ptr);
                av_free(packet_ptr);
            }
        }
        
        if(!m_loops.empty() &&m_loops[ICurrentLoop]) 
        {
            int result = m_loops[ICurrentLoop]->update(max);
            if(result>=0&&!m_syncAV)
            {
                jumpTo(result);
            }
        }
    
    }
    
    
    sws_freeContext(m_sws_ctx);
    av_free(m_buffer);
    av_free(m_pFrameRGB);
    av_free(m_pFrame);
    avcodec_close(m_pCodecCtx);
    avcodec_close(m_paCodecCtx);
    avformat_close_input(&m_pFormatCtx);
    
    delete [] Data;

    return;
}

void MoviePlayer::setFont(sf::Font &font)
{
    m_font = font;
}

void MoviePlayer::addLoop(unsigned int startTime,unsigned int endTime,callme fevent1,const char *text1,callme fevent2,const char *text2)
{
    std::shared_ptr<VideoLoop> loop = std::make_shared<VideoLoop>(startTime,endTime,fevent1,text1,fevent2,text2);
    m_loops.push_back(loop);
}