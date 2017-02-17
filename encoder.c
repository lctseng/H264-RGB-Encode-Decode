/*****************************************************************************
 * example.c: libx264 API usage example
 *****************************************************************************
 * Copyright (C) 2014-2017 x264 project
 *
 * Authors: Anton Mitrofanov <BugMaster@narod.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#ifdef _WIN32
#include <io.h>       /* _setmode() */
#include <fcntl.h>    /* _O_BINARY */
#endif

#include <stdint.h>
#include <stdio.h>
#include <x264.h>

#define FAIL_IF_ERROR( cond, ... )\
do\
{\
    if( cond )\
    {\
        fprintf( stderr, __VA_ARGS__ );\
        goto fail;\
    }\
} while( 0 )


int encoder_init(int width, int height){
  

}

int encoder_cleanup(){
  
}


int encoder_encode(uint8_t* rgb_data, uint8_t** encoded_buf, int* encoded_size){
  
}


int main( int argc, char **argv )
{
    int width, height;
    x264_param_t param;
    x264_picture_t pic;
    x264_picture_t pic_out;
    x264_t *h;
    int i_frame = 0;
    int i_frame_size;
    x264_nal_t *nal;
    int i_nal;


    FAIL_IF_ERROR( !(argc > 1), "Example usage: encoder 352x288 <input.rgb >output.h264\n" );
    FAIL_IF_ERROR( 2 != sscanf( argv[1], "%dx%d", &width, &height ), "resolution not specified or incorrect\n" );

    /* Get default params for preset/tuning */
    //if( x264_param_default_preset( &param, "medium", NULL ) < 0 )
    //    goto fail;

    /* Get default params for preset/tuning */
    x264_param_default(&param);
    /* Configure non-default params */
    // param.i_csp = X264_CSP_I420;
    param.i_csp = X264_CSP_RGB;
    param.i_width  = width;
    param.i_height = height;
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;
    param.b_annexb = 1;
    param.rc.i_qp_constant = 0;
    param.rc.i_rc_method = X264_RC_CQP;

    param.i_fps_num = 50;

    /* Apply profile restrictions. */
    /*
    if( x264_param_apply_profile( &param, "high" ) < 0 )
        goto fail;
    */

    if( x264_picture_alloc( &pic, param.i_csp, param.i_width, param.i_height ) < 0 ){
        goto fail;
    }
#undef fail
#define fail fail2

    h = x264_encoder_open( &param );
    if( !h )
        goto fail;
#undef fail
#define fail fail3

    
    int luma_size = width * height * 3;
    
    fprintf(stderr,"Plane number: %d\n", pic.img.i_plane);
    /* Encode frames */
    for( ;; i_frame++ )
    {
        fprintf(stderr,"Loading frame: %d\n", i_frame);
        /* Read input frame */
        if( fread( pic.img.plane[0], 1, luma_size, stdin ) != luma_size )
            break;

        pic.i_pts = i_frame;
        i_frame_size = x264_encoder_encode( h, &nal, &i_nal, &pic, &pic_out );
        if( i_frame_size < 0 )
            goto fail;
        else if( i_frame_size )
        {
          fprintf(stderr,"Emitting encoded frame, size = %d\n",i_frame_size);
          if( !fwrite( nal->p_payload, i_frame_size, 1, stdout ) )
            goto fail;
        }
    }
    fprintf(stderr,"End of frame input, emitting delayed frame\n");
    /* Flush delayed frames */
    while( x264_encoder_delayed_frames( h ) )
    {
        i_frame_size = x264_encoder_encode( h, &nal, &i_nal, NULL, &pic_out );
        if( i_frame_size < 0 )
            goto fail;
        else if( i_frame_size )
        {
            fprintf(stderr,"[Delayed] Emitting encoded frame, size = %d\n",i_frame_size);
            if( !fwrite( nal->p_payload, i_frame_size, 1, stdout ) )
                goto fail;
        }
    }

    x264_encoder_close( h );
    x264_picture_clean( &pic );
    return 0;

#undef fail
fail3:
    x264_encoder_close( h );
fail2:
    x264_picture_clean( &pic );
fail:
    return -1;
}
