void print(char* m){
    char* c = m;
    char* videoRam = (char*)0xb8000;
    while(*c!='\0'){
        *videoRam++ = *(c++);
        *videoRam++ = 0x0F;
    }
}

void read_disk(int lda, int nSectors, int driveNo, char* des){
}


extern "C" void main(){
    char* msg = "Hello World from C++!";
    print(msg);
    return;
}
