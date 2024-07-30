#ifndef INCLUDE_STREAM_H
#define INCLUDE_STREAM_H
    
#include <cstdint>
#include <type_traits>
#include <utility>

namespace kstd {

    class istream {
    public:
        virtual void ignore(uint32_t n) = 0;
        virtual void seekg(uint32_t pos) = 0;
        virtual uint8_t fail() = 0;
        virtual void read(char* buf, uint32_t size) = 0;
        // virtual void write();
    };  

    template<typename Stream>
    requires std::is_base_of_v<kstd::istream, Stream>
    class istream_offset : istream{
        private:
        std::size_t offset;
        Stream stream;

        public:
        template<typename S1>
        requires std::is_base_of_v<kstd::istream, Stream>
        istream_offset(S1&& stream, std::size_t offset): stream(std::forward<S1>(stream)), offset(offset){
            this->seekg(0);
        }

        void ignore(uint32_t n) override {
            stream.ignore(n);
        }
        void seekg(uint32_t pos) override{
            stream.seekg(pos + offset);
        }
        uint8_t fail() override{
            return stream.fail();
        }
        void read(char* buf, uint32_t size) override{
            stream.read(buf, size);
        }
    };

}


#endif /* INCLUDE_STREAM_H */
    