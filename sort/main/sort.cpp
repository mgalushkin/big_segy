#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <segyio/segy.h>
#include <ctime>
#include <chrono>


#define SAVE_SEGY_FILE_NAME   "save_file.sgy"


struct HeaderSort {
    int _real_number_header{0};
    char _traceheader[SEGY_TRACE_HEADER_SIZE]{0};  
};

bool operator<( const HeaderSort& sort_header_1, const HeaderSort& sort_header_2 ) {
    int sort_param_1 = 0;
    int sort_param_2 = 0;

    segy_get_field( sort_header_1._traceheader, SEGY_TR_SEQ_LINE, &sort_param_1 );
    segy_get_field( sort_header_2._traceheader, SEGY_TR_SEQ_LINE, &sort_param_2 );

    return sort_param_1 > sort_param_2;
}



int main( int argc, char const *argv[] ) {
    if (1 < argc) {
        std::cout << "Reading from " << argv[1] << std::endl;
    } else {
        std::cout << "Usage: " << argv[0] << " filename(segy_file)" << std::endl;
        return -1;
    }

    std::ofstream log_file( "log_file.txt" );
    if (!log_file.is_open()) {
        std::cout << "Unable to open log-file" << std::endl;
        return -1;
    }

    auto time_1 = std::chrono::high_resolution_clock::now();

    segy_file* fp = segy_open( argv[1], "rb" );
    if (!fp) throw std::runtime_error( "segy_open()" );


    char binheader[SEGY_BINARY_HEADER_SIZE]{0};
    int err = segy_binheader( fp, binheader );
    if (SEGY_OK != err) throw std::runtime_error( "segy_binheader()" );

    auto trace0 = segy_trace0( binheader );
    auto format = segy_format( binheader );
    int trace_size = segy_samples( binheader );
    if (0 >= trace_size) throw std::runtime_error( "segy_samples() <= 0" );

    auto trace_bsize = segy_trsize( format, trace_size );
    if (0 >= trace_bsize) throw std::runtime_error( "segy_trsize() <= 0" );

    int trace_amount = 0;
    err = segy_traces( fp, &trace_amount, trace0, trace_bsize );
    if (SEGY_OK != err) throw std::runtime_error( "segy_traces()" );


    log_file << "TRACE_AMOUNT == " << trace_amount << std::endl;
    log_file << "TRACE_SIZE == " << trace_size << std::endl;


    std::vector<HeaderSort> sort_headers;
    sort_headers.reserve(trace_amount);
    for (int traceno = 0; traceno < trace_amount; ++traceno) {
        HeaderSort sort_header;

        int err = segy_traceheader( fp, traceno, sort_header._traceheader, trace0, trace_bsize );
        if (SEGY_OK != err) throw std::runtime_error( "segy_traceheader()" );

        sort_header._real_number_header = traceno;
        
        sort_headers.push_back(sort_header);
    }
    auto time_2 = std::chrono::high_resolution_clock::now();
    std::cout << "reading :: " << std::chrono::duration_cast<std::chrono::seconds>(time_2 - time_1).count() << " sec." << std::endl;
    log_file  << "reading :: " << std::chrono::duration_cast<std::chrono::seconds>(time_2 - time_1).count() << " sec." << std::endl;


    std::sort( sort_headers.begin(), sort_headers.end() );
    auto time_3 = std::chrono::high_resolution_clock::now();
    std::cout << "sorting :: " << std::chrono::duration_cast<std::chrono::seconds>(time_3 - time_2).count() << " sec." << std::endl;
    log_file  << "sorting :: " << std::chrono::duration_cast<std::chrono::seconds>(time_3 - time_2).count() << " sec." << std::endl;


    segy_file* fp_write = segy_open( SAVE_SEGY_FILE_NAME, "wb" );

    segy_write_binheader( fp_write, binheader );
    int traceno = 0;
    for (auto& sort_header : sort_headers) {
        int err = segy_write_traceheader( fp_write, traceno, sort_header._traceheader, trace0, trace_bsize );
        if (SEGY_OK != err) throw std::runtime_error( "segy_write_traceheader()" );

        uint32_t buffer_size = static_cast<uint32_t>(trace_bsize) / sizeof(float);
        float *buffer = new float[buffer_size];
        err = segy_readtrace( fp, sort_header._real_number_header, buffer, trace0, trace_bsize );
        if (SEGY_OK != err) throw std::runtime_error( "segy_readtrace()" );

        segy_writetrace( fp_write, traceno, buffer, trace0, trace_bsize );
        if (SEGY_OK != err) throw std::runtime_error( "segy_writetrace()" );

        ++traceno;

        delete[] buffer;
    }
    auto time_4 = std::chrono::high_resolution_clock::now();
    std::cout << "writing :: " << std::chrono::duration_cast<std::chrono::seconds>(time_4 - time_3).count() << " sec." << std::endl;
    log_file  << "writing :: " << std::chrono::duration_cast<std::chrono::seconds>(time_4 - time_3).count() << " sec." << std::endl;



    segy_close( fp );
    segy_close( fp_write );
    return 0;
}
