#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if(data.empty()){
    if(is_last_substring){
      output_.writer().close();
    }
    return;
  }
  if(writer().available_capacity() == 0){
    return;
  }
  uint64_t end_ind = first_index + data.size();
  uint64_t unaccepted = first_un_pushed_idx_ + writer().available_capacity();
  if(end_ind <= first_un_pushed_idx_ || first_index >= unaccepted){
    return;
  }
  if(end_ind > unaccepted){
    data = data.substr(0, unaccepted - first_index);
    is_last_substring = false;
  }
  if(first_index > first_un_pushed_idx_){
    uint64_t begin_index = first_index;
    uint64_t end_index = begin_index + data.size();
    for(auto it = buffer_.begin(); (it != buffer_.end()) && (begin_index < end_index); ){
      if(it->first <= begin_index){
        begin_index = std::max(begin_index, it->first + it->second.size());
        it++;
        continue;
      }
      if(begin_index == first_index && end_index <= it->first){
        buffer_size_ += data.size();
        buffer_.emplace(it, first_index, std::move(data));
        return;
      }
      uint64_t right_index = std::min(it->first, end_index);
      uint64_t length = right_index - begin_index;
      buffer_.emplace(it, begin_index, data.substr(begin_index - first_index, length));
      buffer_size_ += length;
      begin_index = right_index;
    }
    if(begin_index < end_index){
      buffer_size_ += (end_index - begin_index);
      buffer_.emplace_back(begin_index, data.substr(begin_index - first_index, end_index - begin_index));
    }
    if(is_last_substring){
      has_last_ = true;
    }
    return;
  }
  if(first_index < first_un_pushed_idx_){
    data = data.substr(first_un_pushed_idx_ - first_index);
  }
  first_un_pushed_idx_ += data.size();
  output_.writer().push(std::move(data));

  if(is_last_substring){
    output_.writer().close();
  }
  if(!buffer_.empty() && buffer_.begin()->first <= first_un_pushed_idx_){
    for(auto it = buffer_.begin(); it != buffer_.end();){
      if(it->first > first_un_pushed_idx_){
        break;
      }
      uint64_t end_index = it->first + it->second.size();
      if(end_index <= first_un_pushed_idx_){
        buffer_size_ -= it->second.size();
      }else{
        auto newdata = std::move(it->second);
        buffer_size_ -= newdata.size();
        if(it->first < first_un_pushed_idx_){
          newdata = newdata.substr(first_un_pushed_idx_ - it->first);
        }
        first_un_pushed_idx_ += newdata.size();
        output_.writer().push(std::move(newdata));
      }
      it = buffer_.erase(it);
    }
    if(buffer_.empty() && has_last_){
      output_.writer().close();
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return buffer_size_;
}
