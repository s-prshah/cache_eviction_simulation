#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <map> 
#include <cstring> 

#include "parameter.h"
#include "executor.h"
#include "workload_generator.h"

using namespace std;
using namespace bufmanager;

Buffer *Buffer::buffer_instance;
long Buffer::max_buffer_size = 0;
int Buffer::buffer_hit = 0;
int Buffer::buffer_miss = 0;
int Buffer::read_io = 0;
int Buffer::write_io = 0;
int Buffer::read_perc = 0; 
int Buffer::write_perc = 0; 

int Buffer::time_stamp_num = 0; 
int Buffer::buffer_capacity = 0; 
int Buffer::buffer_hand = 0; // added for the SIEVE algorithm 
int Buffer::sifting_count = 0; // added for the SIEVE algorithm 
int Buffer::cf_pointer = -1; // added for the CFLRU algorithm; is inclusive (i.e, if it's 6, 6 is included in the index)
int Buffer::cf_portion = -1; // added for the CFLRU algorithm 

// create a workload object
Page::Page(int page_id, std::string contents)
{
  this->_page_id = page_id; 
  this->_contents = contents; 
  this->is_dirty = false; 
  this->time_stamp = 0; 
  this->visited = false; 
}

Buffer::Buffer(Simulation_Environment *_env)
{
  std::vector<Page> bufferpool = {};
  this->buffer_capacity = _env->buffer_size_in_pages;
  this->cf_portion = _env->window_size; 
  this->read_perc = _env->perct_reads; 
  this->write_perc = _env->perct_writes; 
}

//creates a pointer called buffer of the buffer class -> inside, there is a function called "getBufferInstance"
// :: is just used to define this function outside of where the Buffer class was initialized 

Buffer *Buffer::getBufferInstance(Simulation_Environment *_env)
{
  if (buffer_instance == 0) {
    buffer_instance = new Buffer(_env);
  }
  return buffer_instance;
}

Page Buffer::fetch_page(int page_id) { 
  const char value_alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  char* contents = new char[4096]; 
  for(int n = 0; n < 4096; n++) {
    int randNum = rand()%(52);
    contents[n] = value_alphanum[randNum]; 
  }
  Page r_page(page_id, contents);
  return r_page; 
}

void Buffer::replace_contents(int start, std::string to_replace, Page r_page)
{
  int str_index = 0;
  for(int i = start; i < (start + 128); i++) {
    r_page._contents[i] = to_replace[str_index]; 
    str_index++; 
  }
  //to do : memcopy
}


std::string Buffer::read_contents(int start, Page r_page)
{
  std::string to_return; 
  for(int i = start; i < (start + 128); i++) {
    to_return.push_back((r_page._contents[i])); 
  }
  return to_return; 
}

int Buffer::id_to_index(std::vector<Page> buffer, int pageId) {
  for(auto i = buffer.begin(); i != buffer.end(); ++i) {
    int index = i - buffer.begin(); 
    if(buffer[index]._page_id == pageId)
    {
      return index; 
    }
  }
  return -1; 
}

//finds first clean page
int Buffer::find_cf(std::vector<Page> buffer, int window) {
  int t_index = 0; 
  while(t_index <= window && buffer[t_index].is_dirty == false) {
    if(t_index == window) {
      return 0; 
    }
    t_index++; 
  }
  return t_index; 
}

int WorkloadExecutor::search(Buffer* buffer_instance, int pageId)
{
  //std::cout << "Searching for page " << pageId << std::endl;
  //std::cout << "Current buffer size: " << buffer_instance->bufferpool.size() << std::endl;
  // Implement Search in the Bufferpool
  for(auto p = buffer_instance->bufferpool.begin(); p != buffer_instance->bufferpool.end(); ++p){
    int p_index = p - buffer_instance->bufferpool.begin(); 
    if(pageId == buffer_instance->bufferpool[p_index]._page_id)
      return p_index; 
  }
  return -1;
}

int WorkloadExecutor::read(Buffer* buffer_instance, int pageId, int offset, int algorithm)
{
  // Implement Read in the Bufferpool
  // std::cout << "Reading pageid " << pageId << std::endl; 
  int id = search(buffer_instance, pageId);

  //std::cout<<"The algorithm is "<<algorithm<<std::endl; 

  if (id > -1) {
    //std::cout<<"Buffer hit has occured in the read method."<<std::endl; 
    buffer_instance->buffer_hit += 1;
    buffer_instance->bufferpool[id].time_stamp = buffer_instance->time_stamp_num; 
    buffer_instance->time_stamp_num += 1; 
    std:: string to_read = buffer_instance->read_contents((offset * 128), buffer_instance->bufferpool[id]);

    // additional steps for if the algorithm is LRU or CFLRU 
    if(algorithm == 0 || algorithm == 1) {
      // shift vector
      Page t_page = buffer_instance->bufferpool[id]; 
      buffer_instance->bufferpool.erase(buffer_instance->bufferpool.begin() + id); 
      buffer_instance->bufferpool.push_back(t_page); 
    }
    // additional steps for if the algorithm is SIEVE
    if(algorithm == 2) {
      buffer_instance->bufferpool[id].visited = true; 
    }
    return id; 
  }
  
  buffer_instance->buffer_miss += 1; 
  buffer_instance->read_io += 1; 
  Page cur_page = buffer_instance->fetch_page(pageId); 

  switch(algorithm) 
  {
    case 0:
      buffer_instance->LRU(cur_page); 
      break; 
    case 1:
      // false means that this is a read function 
      buffer_instance->CFLRU(cur_page, false); 
      break; 
    case 2: 
      buffer_instance->SIEVE(cur_page); 
      break; 
    default:
      std::cout<<"The algorithm is not applicable."<<std::endl; 
  }

  std:: string to_read = buffer_instance->read_contents((offset * 128), cur_page); 
  return -1;

}

int WorkloadExecutor::write(Buffer* buffer_instance, int pageId, int offset, const string new_entry, int algorithm)
{
  //std::cout << "Writing pageid " << pageId << std::endl;
  // Implement Read in the Bufferpool
  int id = search(buffer_instance, pageId);
  if (id > -1) {
    //std::cout<<"Buffer hit has occured in the write method."<<std::endl; 
    buffer_instance->buffer_hit += 1; 
    buffer_instance->bufferpool[id].time_stamp = buffer_instance->time_stamp_num; 
    buffer_instance->time_stamp_num += 1; 
    // update/add to the string "contents"
    buffer_instance->replace_contents((offset*128), new_entry, buffer_instance->bufferpool[id]); 
    buffer_instance->bufferpool[id].is_dirty = true; 

    // additional steps for if the algorithm is LRU or CFLRU
    if(algorithm == 0 || algorithm == 1) {
      Page t_page = buffer_instance->bufferpool[id]; 
      buffer_instance->bufferpool.erase(buffer_instance->bufferpool.begin() + id); 
      buffer_instance->bufferpool.push_back(t_page); 
    }

    // additional steps for if the algorithm is SIEVE
    if(algorithm == 2) {
      buffer_instance->bufferpool[id].visited = true; 
    }
    return id; 
  }
  buffer_instance->buffer_miss += 1; 
  buffer_instance->write_io += 1; 
  //std::cout<<"The buffer miss is now: " << buffer_instance->buffer_miss << ", and the write_io is now: " << buffer_instance->write_io <<std::endl; 
  Page cur_page = buffer_instance->fetch_page(pageId); 
  //std::cout<<"***Page Contents: "<<endl<< "is_dirty: "<< cur_page.is_dirty<<endl<<"id: "<<cur_page._page_id<<endl<<"time stamp: "<<cur_page.time_stamp<<"***"<<endl; 
  
  switch(algorithm) 
  {
    case 0:
      buffer_instance->LRU(cur_page); 
      break; 
    case 1:
      // true means that this is a write function 
      buffer_instance->CFLRU(cur_page, true); 
      break; 
    case 2: 
      buffer_instance->SIEVE(cur_page); 
      break; 
    default:
      std::cout<<"The algorithm is not applicable."<<std::endl; 
  }

  int id_index = buffer_instance->id_to_index(buffer_instance->bufferpool, pageId); 
  
  buffer_instance->replace_contents((offset*128), new_entry, cur_page); 
  buffer_instance->bufferpool[id_index].is_dirty = true; 

  return -1;
};


int Buffer::LRU(Page page)
{
  // remove current page from referenced pages 
  // update the time stamp for the page (for tracking LRU)
  page.time_stamp = buffer_instance->time_stamp_num; 
  //std::cout<<"Time Stamp is: "<<page.time_stamp<<endl;
  //std::cout << "buffer_instance->bufferpool.size() = " << buffer_instance->bufferpool.size() << std::endl;
  if(buffer_instance->bufferpool.size() >= buffer_instance->buffer_capacity) {
    // remove least recently used page from the bufferpool 
    buffer_instance->bufferpool.erase(buffer_instance->bufferpool.begin());
  }
  buffer_instance->bufferpool.push_back(page); 
  // update ongoing highest time stamp 
  buffer_instance->time_stamp_num += 1; 
  //std::cout << "The overall running time stamp is now " << buffer_instance->time_stamp_num << std::endl;
  return -1; //change what this returns to the index at which the algorithm got replaced at first 
};


// CFLRU isn't working the way it should because in general, the larger the window size of the clean-first region,
// the smaller the miss rate is (theoretically, it should be the opposite)
int Buffer::CFLRU(Page page, bool type) 
{
  // remove current page from referenced pages 
  // update the time stamp for the page (for tracking LRU)
  page.time_stamp = buffer_instance->time_stamp_num; 
  //std::cout<<"Time stamp for this page: "<<page.time_stamp<<std::endl; 
  //std::cout<<"Bufferpool size: "<<buffer_instance->bufferpool.size()<<std::endl; 
  buffer_instance->cf_pointer = ((buffer_instance->bufferpool.size()) / (buffer_instance->cf_portion)); 
  //std::cout<<"Window size: "<<buffer_instance->cf_pointer<<endl; 

  if(buffer_instance->bufferpool.size() < buffer_instance->buffer_capacity) {
    // unsure about this 
    buffer_instance->bufferpool.push_back(page); 
  }
  else {
    // find a more efficient way to find the first clean page
    int to_remove = buffer_instance->find_cf(buffer_instance->bufferpool, buffer_instance->cf_pointer); 
    buffer_instance->bufferpool.erase((buffer_instance->bufferpool.begin() + to_remove));
    buffer_instance->bufferpool.push_back(page);
  }

  buffer_instance->time_stamp_num++; 
  //std::cout<<"The time stamp has been updated to "<<buffer_instance->time_stamp_num<<" through the CFLRU policy."<<std::endl; 
  return -1; //change what this returns to the index at which the algorithm got replaced at first 
}

int Buffer::SIEVE(Page page)
{
  page.time_stamp = buffer_instance->time_stamp_num; 
  //std::cout<<"Time stamp for this page: "<<page.time_stamp<<std::endl; 
  //std::cout<<"Bufferpool size = "<<buffer_instance->bufferpool.size()<<std::endl; 
  // remove the current page from the array of referenced/requested pages

  if(buffer_instance->bufferpool.size() < buffer_instance->buffer_capacity) {
    buffer_instance->bufferpool.push_back(page); 
  }
  else {
    // remove the current page from the list referenced/requested pages 
    while(buffer_instance->bufferpool[buffer_instance->buffer_hand].visited == true) {
      buffer_instance->bufferpool[buffer_instance->buffer_hand].visited = false; 
      if((buffer_instance->buffer_hand + 1) == buffer_instance->buffer_capacity) {
        buffer_instance->buffer_hand = 0; 
        buffer_instance->sifting_count++; 
      }
      else {
        buffer_instance->buffer_hand++; 
      }
    }
    buffer_instance->bufferpool.erase((buffer_instance->bufferpool.begin() + buffer_instance->buffer_hand)); 
    buffer_instance->bufferpool.push_back(page); 
  }

  buffer_instance->time_stamp_num += 1; 
  //std::cout<<"Sieve algorithm is updating the overall time stamp; it is now "<<buffer_instance->time_stamp_num<<std::endl; 
  
  return -1; // change this to whatever the SIEVE algorithm should actually return 
}

int Buffer::printBuffer()
{
  return -1;
}

int Buffer::printStats()
{
  Simulation_Environment* _env = Simulation_Environment::getInstance();
  cout << "******************************************************" << endl;
  cout << "Printing Stats..." << endl;
  cout << "Number of operations: " << _env->num_operations << endl;
  cout << "Buffer Hit: " << buffer_hit << endl;
  cout << "Buffer Miss: " << buffer_miss << endl;
  cout << "Buffer Size: " << buffer_capacity << endl; 
  cout << "Read Percentage: " << read_perc << endl; 
  cout << "Write Percentage: " << write_perc << endl; 
  cout << "Read IO: " << read_io << endl;
  cout << "Write IO: " << write_io << endl;  
  cout << "Global Clock: " << endl;
  cout << "******************************************************" << endl;
  return 0;
}
