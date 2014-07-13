/** 
 * PUBLIC HEADER
 *
 * Akumuli API.
 * Contains only POD definitions that can be used from "C" code.
 *
 * Copyright (c) 2013 Eugene Lazin <4lazin@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <cstdint>
#include <apr_errno.h>
#include "config.h"
#include "akumuli_def.h"

extern "C" {

    typedef uint64_t aku_TimeStamp;
    typedef uint64_t aku_Duration;
    typedef uint32_t aku_EntryOffset;
    typedef uint32_t aku_ParamId;
    typedef int      aku_Status;

    struct aku_MemRange {
        void* address;
        uint32_t length;
    };

    struct aku_Entry {
        aku_TimeStamp  time;      //< Entry timestamp
        aku_ParamId    param_id;  //< Parameter ID
        uint32_t       length;    //< Entry length: constant + variable sized parts
        uint32_t       value[];   //< Data begining
    } __attribute__((packed));

    const char* aku_error_message(int error_code);

    //! Database instance.
    struct aku_Database {
    };

    /**
     * @brief Creates storage for new database on the hard drive
     * @param file_name database file name
     * @param metadata_path path to metadata file
     * @param volumes_path path to volumes
     * @param num_volumes number of volumes to create
     * @return APR errorcode or APR_SUCCESS
     */
    apr_status_t create_database( const char* 	file_name
                                , const char* 	metadata_path
                                , const char* 	volumes_path
                                , int32_t       num_volumes
                                );

    /**
     * @brief Select search query.
     */
    struct aku_SelectQuery {
        //! Begining of the search range
        aku_TimeStamp begin;
        //! End of the search range
        aku_TimeStamp end;
        //! Number of parameters to search
        uint32_t n_params;
        //! Array of parameters to search
        aku_ParamId params[];
    };

    /**
     * @brief The aku_Cursor struct
     */
    struct aku_Cursor {
    };

    /**
     * @brief Create select query
     */
    aku_SelectQuery* aku_make_select_query(aku_TimeStamp begin, aku_TimeStamp end, uint32_t n_params, aku_ParamId* params);

    /**
     * @brief Destroy any object created with aku_make_*** function
     */
    void aku_destroy(void* any);

    /**
     * @brief Execute query
     * @param query data structure representing search query
     * @return cursor
     */
    aku_Cursor* aku_select(aku_Database* db, aku_SelectQuery* query);

    /**
     * @brief Close cursor
     * @param pcursor pointer to cursor
     */
    void aku_close_cursor(aku_Cursor* pcursor);

    int aku_cursor_read(aku_Cursor* pcursor, aku_Entry const** buffer, int buffer_len);

    bool aku_cursor_is_done(aku_Cursor* pcursor);

    bool aku_cursor_is_error(aku_Cursor* pcursor, int* out_error_code_or_null);

    /**
     * @brief Flush data to disk.
     * @param db database.
     */
    void aku_flush_database(aku_Database* db);


    /** Open existing database.
     */
    aku_Database* aku_open_database(const char *path, aku_Config config);

    aku_Status aku_add_sample(aku_Database* db, aku_ParamId param_id, aku_TimeStamp long_timestamp, aku_MemRange value);


    /** Close database.
     */
    void aku_close_database(aku_Database* db);
}
