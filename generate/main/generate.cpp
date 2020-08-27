#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <segyio/segy.h>


#define TRACE_AMOUNT    1000000
#define TRACE_SIZE      2000
#define SAMPLE_INTERVAL 250

#define SAVE_SEGY_FILE_NAME  "big_segy_file.sgy"


long write_binheader( segy_file* );
float* generateTraceData( int );


int main() {
    segy_file* fp = segy_open( SAVE_SEGY_FILE_NAME, "wb" );
    if (!fp) throw std::runtime_error( "segy_open()" );

    auto trace0 = write_binheader( fp );
    auto trace_bsize = segy_trace_bsize( TRACE_SIZE );

    int err = SEGY_OK;
    // std::srand( time( nullptr ) );
    // bool check[TRACE_AMOUNT]{false};
    for (int traceno = 0; traceno < TRACE_AMOUNT; ++traceno) {
        char trace_header[SEGY_TRACE_HEADER_SIZE] = {0};

        // int rand = 0;
        // while (true) {
        //     rand = std::rand() % TRACE_AMOUNT;
        //     if (!check[rand]) {
        //         check[rand] = true;
        //         break;
        //     }
        // }
        err = segy_set_field( trace_header, SEGY_TR_SEQ_LINE, traceno+1 );
        if (SEGY_OK != err) throw std::runtime_error( "segy_set_field(SEGY_TR_SEQ_LINE)" );

        err = segy_set_field( trace_header, SEGY_TR_TRACE_ID, 1 );
        if (SEGY_OK != err) throw std::runtime_error( "segy_set_field(SEGY_TR_TRACE_ID)" );

        err = segy_set_field( trace_header, SEGY_TR_SAMPLE_INTER, SAMPLE_INTERVAL );
        if (SEGY_OK != err) throw std::runtime_error( "segy_set_field(SEGY_TR_SAMPLE_INTER)" );

        err = segy_write_traceheader( fp, traceno, trace_header, trace0, trace_bsize );
        if (SEGY_OK != err) throw std::runtime_error( "segy_write_traceheader()" );

        float* trace_data = generateTraceData( TRACE_SIZE );
        segy_from_native( SEGY_IEEE_FLOAT_4_BYTE, TRACE_AMOUNT, trace_data );
        err = segy_writetrace( fp, traceno, trace_data, trace0, trace_bsize );
        if (SEGY_OK != err) throw std::runtime_error( "segy_writetrace()" );

        delete[] trace_data;
    }

    segy_close( fp );
    return 0;
}


long write_binheader( segy_file* fp ) {
    int err = SEGY_OK;
    char binheader[SEGY_BINARY_HEADER_SIZE] = {0};

    err = segy_set_bfield( binheader, SEGY_BIN_TRACES, TRACE_AMOUNT );
    if (SEGY_OK != err) throw std::runtime_error( "segy_set_bfield(SEGY_BIN_TRACES)" );

    err = segy_set_bfield( binheader, SEGY_BIN_INTERVAL, SAMPLE_INTERVAL );
    if (SEGY_OK != err) throw std::runtime_error( "segy_set_bfield(SEGY_BIN_INTERVAL)" );

    err = segy_set_bfield( binheader, SEGY_BIN_SAMPLES, TRACE_SIZE );
    if (SEGY_OK != err) throw std::runtime_error( "segy_set_bfield(SEGY_BIN_SAMPLES)" );

    err = segy_set_bfield( binheader, SEGY_BIN_FORMAT, SEGY_IEEE_FLOAT_4_BYTE );
    if (SEGY_OK != err) throw std::runtime_error( "segy_set_bfield(SEGY_BIN_FORMAT)" );

    err = segy_set_bfield( binheader, SEGY_BIN_TRACE_FLAG, 1 );
    if (SEGY_OK != err) throw std::runtime_error( "segy_set_bfield(SEGY_BIN_TRACE_FLAG)" );

    err = segy_write_binheader( fp, binheader );
    if (SEGY_OK != err) throw std::runtime_error( "segy_write_binheader()" );

    return segy_trace0( binheader );
}

float* generateTraceData( int trace_size ){
    float* trace_data = new float[trace_size];
    for (int i = 0; i < trace_size; ++i) {
        trace_data[i] = i;
    }

    return trace_data;
}
