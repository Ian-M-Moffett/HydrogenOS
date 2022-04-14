#define STB_IMAGE_IMPLEMENTATION
#include <uefi.h>
#include <stdint.h>
#include <stb_img.h>

efi_gop_t* gop;


void setup_framebuffer() {
    efi_status_t status;
    efi_guid_t gopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    gop = NULL;

    status = BS->LocateProtocol(&gopGUID, NULL, (void**)&gop);

    /* Setup video mode. */
    if (!(EFI_ERROR(status)) && gop) {
        status = gop->SetMode(gop, 0);
        ST->ConOut->Reset(ST->ConOut, 0);
        ST->StdErr->Reset(ST->StdErr, 0);

        if (EFI_ERROR(status)) {
            printf("Unable to set video mode.\n");
        }
    } else {
        printf("Unable to set video mode.\n");
    }
}


void show_bootscreen() {
    unsigned char* buffer;
    uint32_t* data;
    int w, h, l;
    long int size;
    stbi__context s;
    stbi__result_info ri;

    FILE* fp = fopen("\\rb.png", "r");

    if (fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = (unsigned char*)malloc(size);

        if (!(buffer)) {
            printf("Unable to allocate memory for rb.png\n");
            return;
        }

        fread(buffer, size, sizeof(char), fp);
        fclose(fp);
        ri.bits_per_channel = 8;
        s.read_from_callbacks = 0;
        s.img_buffer = s.img_buffer_original = buffer;
        s.img_buffer_end = s.img_buffer_original = buffer + size;
        data = (uint32_t*)stbi__png_load(&s, &w, &h, &l, 4, &ri);

        if (!(data)) {
            printf("Unable to decode rb.png\n");
            return;
        }
    } else {
        printf("Unable to open rb.png\n");
    }

    /* PNG is RGBA but UEFI is using BGRA. */
    if(gop->Mode->Information->PixelFormat == PixelBlueGreenRedReserved8BitPerColor || (gop->Mode->Information->PixelFormat == PixelBitMask && gop->Mode->Information->PixelInformation.BlueMask != 0xff0000)) {
        for(l = 0; l < w * h; l++)
            data[l] = ((data[l] & 0xff) << 16) | (data[l] & 0xff00) | ((data[l] >> 16) & 0xff);
    }

    /* Display the image. */
    gop->Blt(gop, data, EfiBltBufferToVideo, 0, 0, (gop->Mode->Information->HorizontalResolution - w) / 2,
        (gop->Mode->Information->VerticalResolution - h) / 2, w, h, 0);

}


int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    setup_framebuffer();
    show_bootscreen();
    printf("Booting Hydrogen OS..\n");
    __asm__ __volatile__("cli; hlt");
} 
