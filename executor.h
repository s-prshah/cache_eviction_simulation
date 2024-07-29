 
#include "parameter.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

namespace bufmanager {
  class Page{
    private:
    public: 
    Page(int page_id, std::string contents);

    int _page_id;
    std::string _contents;
    bool is_dirty;
    int time_stamp;
    bool visited;

  };

  
  class Buffer {
    // This class maintains specific property of the buffer.
    // You definitely need to modify this part
    // You need to add more variables here for your implementation. For example, currently the bufferpool itself is missing

    private:
    Buffer(Simulation_Environment* _env);
    static Buffer* buffer_instance;

    public:
    static long max_buffer_size;  //in pages
    
    static Buffer* getBufferInstance(Simulation_Environment* _env);

    static int buffer_hit;
    static int buffer_miss;
    static int read_io;
    static int write_io;

    static int time_stamp_num;  
    static int buffer_capacity; 
    static int read_perc; 
    static int write_perc; 
    static int buffer_hand; // added for the SIEVE algorithm 
    static int sifting_count; // added for the SIEVE algorithm 
    static int cf_pointer; // added for the CFLRU algorithm 
    static int cf_portion; // added for the CFLRU algorithm 


    Page fetch_page(int page_id); 
    int LRU(Page page);
    int CFLRU(Page page, bool type);
    int SIEVE(Page page);

    int id_to_index(std::vector<Page> buffer, int pageId); 
    void replace_contents(int start, std::string to_replace, Page r_page); 
    std::string read_contents(int start, Page r_page); 
    int find_cf(std::vector<Page> buffer, int window); 

    static int printBuffer();
    static int printStats();
    
    std::vector<Page> bufferpool = {}; 
    
  };

  class Disk {
    private: 
    public:
  };

  class WorkloadExecutor {
    private:
    public:
    static int read(Buffer* buffer_instance, int pageId, int offset, int algorithm);
    static int write(Buffer* buffer_instance, int pageId, int offset, const string new_entry, int algorithm);
    static int search(Buffer* buffer_instance, int pageId);
    static int unpin(Buffer* buffer_instance, int pageId);
  };
}
