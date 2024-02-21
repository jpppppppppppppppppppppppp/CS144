#include "byte_stream.hh"
#include <bits/stdint-uintn.h>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
    return is_close;
}

void Writer::push( string data )
{
  if(available_capacity() == 0 || data.empty()){
    return;
  }
  uint64_t size_to_push = std::min(data.size(), available_capacity());
  if (data.size() > size_to_push) {
    data = data.substr(0, size_to_push);
  }
  data_stream.push_back(std::move(data));
  data_view.emplace_back(data_stream.back().c_str(), size_to_push);
  num_byted_buffer += size_to_push;
  num_bytes_pushed += size_to_push;
}

void Writer::close()
{
  is_close = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - num_byted_buffer;
}

uint64_t Writer::bytes_pushed() const
{
  return num_bytes_pushed;
}

bool Reader::is_finished() const
{
  return is_close && (num_byted_buffer == 0);
}

uint64_t Reader::bytes_popped() const
{
  return num_bytes_popped;
}

string_view Reader::peek() const
{
  if(data_view.empty()){
    return {};
  }
  return data_view.front();
}

void Reader::pop( uint64_t len )
{
  uint64_t size_to_pop = std::min(len, num_byted_buffer);
  while(size_to_pop > 0){
    uint64_t size_front = data_view.front().size();
    if(size_to_pop < size_front){
      data_view.front().remove_prefix(size_to_pop);
      num_byted_buffer -= size_to_pop;
      num_bytes_popped += size_to_pop;
      return;
    }
    data_view.pop_front();
    data_stream.pop_front();
    size_to_pop -= size_front;
    num_byted_buffer -= size_front;
    num_bytes_popped += size_front;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return num_byted_buffer;
}
