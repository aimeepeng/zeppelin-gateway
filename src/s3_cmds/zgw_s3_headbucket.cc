#include "src/s3_cmds/zgw_s3_bucket.h"

#include "slash/include/env.h"
#include "src/zgw_utils.h"

bool HeadBucketCmd::DoInitial() {
  http_response_xml_.clear();

  return TryAuth();
}

void HeadBucketCmd::DoAndResponse(pink::HttpResponse* resp) {
  if (http_ret_code_ == 200) {
    Status s = store_->GetBucket(user_name_, bucket_name_, &bucket_);
    if (s.ok()) {
      // Just response 200 OK
    } else if (s.ToString().find("Bucket Doesn't Belong To This User") ||
               s.ToString().find("Bucket Not Found")) {
      http_ret_code_ = 404;
      GenerateErrorXml(kNoSuchBucket, bucket_name_);
      resp->SetContentLength(http_response_xml_.size());
    } else {
      http_ret_code_ = 500;
    }
  }

  resp->SetHeaders("Date", http_nowtime(slash::NowMicros()));
  resp->SetStatusCode(http_ret_code_);
}

int HeadBucketCmd::DoResponseBody(char* buf, size_t max_size) {
  if (max_size < http_response_xml_.size()) {
    memcpy(buf, http_response_xml_.data(), max_size);
    http_response_xml_.assign(http_response_xml_.substr(max_size));
  } else {
    memcpy(buf, http_response_xml_.data(), http_response_xml_.size());
  }
  return std::min(max_size, http_response_xml_.size());
}
