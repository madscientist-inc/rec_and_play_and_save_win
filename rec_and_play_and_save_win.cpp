#define _CRT_SECURE_NO_WARNINGS // fopen�֐��̔񐄏��x����}������
#include <Windows.h>
#include <string> // std::string�̂��߂�<string>�w�b�_�[�t�@�C�����C���N���[�h
#include <cstdio> // fopen, fwrite, fclose���g�p���邽�߂�<cstdio>���C���N���[�h
#include <cmath>
#include <vector>
#include <iostream>
#pragma comment(lib, "Winmm.lib") // Winmm.lib�������N����w��

const double PI = 3.14159265358979323846;

const int SAMPLE_RATE = 44100;  // �T���v�����[�g�iHz�j
const int BITS_PER_SAMPLE = 16; // �T���v��������̃r�b�g��
const int CHANNELS = 1;         // �`�����l�����i���m�����j
const int RECORD_DURATION = 5; // �^�����ԁi�b�j

// �^�������f�[�^��WAV�t�@�C���ɕۑ�����
void saveToWavFile2(const std::vector<short>& buffer, const std::string& filename) {
    FILE* file = nullptr;
    if (fopen_s(&file, filename.c_str(), "wb") == 0 && file != nullptr) {
        // WAV�t�@�C���̃w�b�_�[����ݒ�
        int dataBytes = static_cast<int>(buffer.size() * sizeof(short));
        int fileSize = 36 + dataBytes;
        int byteRate = SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8);

        fwrite("RIFF", 1, 4, file);
        fwrite(&fileSize, sizeof(int), 1, file);
        fwrite("WAVEfmt ", 1, 8, file);
        int formatSize = 16;
        fwrite(&formatSize, sizeof(int), 1, file);
        short audioFormat = 1; // PCM
        fwrite(&audioFormat, sizeof(short), 1, file);
        fwrite(&CHANNELS, sizeof(short), 1, file);
        fwrite(&SAMPLE_RATE, sizeof(int), 1, file);
        fwrite(&byteRate, sizeof(int), 1, file);
        short blockAlign = CHANNELS * (BITS_PER_SAMPLE / 8);
        fwrite(&blockAlign, sizeof(short), 1, file);
        fwrite(&BITS_PER_SAMPLE, sizeof(short), 1, file);
        fwrite("data", 1, 4, file);
        fwrite(&dataBytes, sizeof(int), 1, file);

        // �����f�[�^����������
        fwrite(buffer.data(), sizeof(short), buffer.size(), file);

        fclose(file);
        std::cout << "�t�@�C���ۑ�" << std::endl;
    }
}

// .wav�t�@�C���Ƀf�[�^��ۑ�����֐�
void saveToWavFile(const char* outputFileName, char* buffer, DWORD bufferSize, int sampleRate, int channels, int bitsPerSample) {
    // .wav�t�@�C���w�b�_�̏���ݒ�
    WAVEFORMATEX wavHeader;
    memset(&wavHeader, 0, sizeof(WAVEFORMATEX));
    wavHeader.wFormatTag = WAVE_FORMAT_PCM;
    wavHeader.nChannels = channels;
    wavHeader.nSamplesPerSec = sampleRate;
    wavHeader.nAvgBytesPerSec = sampleRate * channels * bitsPerSample / 8;
    wavHeader.nBlockAlign = channels * bitsPerSample / 8;
    wavHeader.wBitsPerSample = bitsPerSample;
    wavHeader.cbSize = 0;

    // .wav�t�@�C�����쐬���ď�������
    FILE* file = fopen(outputFileName, "wb");
    if (file == nullptr) {
        printf("Error opening output file.\n");
        return;
    }

    // .wav�t�@�C���w�b�_����������
    fwrite("RIFF", 1, 4, file);
    DWORD fileSize = bufferSize + sizeof(WAVEFORMATEX) + sizeof(WORD) + sizeof(DWORD) - 8;
    fwrite(&fileSize, sizeof(DWORD), 1, file);
    fwrite("WAVE", 1, 4, file);
    fwrite("fmt ", 1, 4, file);
    DWORD formatSize = sizeof(WAVEFORMATEX);
    fwrite(&formatSize, sizeof(DWORD), 1, file);
    fwrite(&wavHeader, sizeof(WAVEFORMATEX), 1, file);
    fwrite("data", 1, 4, file);
    fwrite(&bufferSize, sizeof(DWORD), 1, file);

    // �^�������f�[�^���t�@�C���ɏ�������
    fwrite(buffer, 1, bufferSize, file);

    // �t�@�C�������
    fclose(file);
    std::cout << "WAV�t�@�C���ۑ�" << std::endl;
}


// �T�C���g�𐶐�����֐�
void generateSineWave(std::vector<short>& buffer, double frequency, double duration, int sampleRate) {
    double amplitude = 3000;
    double time = 0.0;
    double increment = 1.0 / sampleRate;

    for (size_t i = 0; i < buffer.size(); ++i) {
        buffer[i] = static_cast<short>(amplitude * sin(2.0 * PI * frequency * time));
        time += increment;
    }
}


void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    // Do nothing, we are just using this callback to signal the recording is finished.
}

int main() {
    // �ݒ�
    const int duration = 10; // �^���E�Đ����� (�b)
    const int sampleRate = 44100; // �T���v�����[�g (Hz)
    const int bitsPerSample = 16; // �ʎq���r�b�g�� (16�r�b�g)
    const int channels = 1; // ���m����

    // �^������
    HWAVEIN hWaveIn;
    WAVEFORMATEX wfx;
    wfx.cbSize = sizeof(WAVEFORMATEX);
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = channels;
    wfx.nSamplesPerSec = sampleRate;
    wfx.wBitsPerSample = bitsPerSample;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    MMRESULT result = waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION);
    if (result) {
        printf("Error opening wave input device: %u\n", result);
        return 1;
    }

    // �^���o�b�t�@�̏���
    const int bufferSize = duration * sampleRate * wfx.nBlockAlign;
    char* buffer = new char[bufferSize];
    WAVEHDR waveHdr;
    waveHdr.lpData = buffer;
    waveHdr.dwBufferLength = bufferSize;
    waveHdr.dwBytesRecorded = 0;
    waveHdr.dwUser = 0;
    waveHdr.dwFlags = 0;
    waveHdr.dwLoops = 0;

    // �^���J�n
    result = waveInPrepareHeader(hWaveIn, &waveHdr, sizeof(WAVEHDR));
    if (result) {
        printf("Error preparing wave input header: %u\n", result);
        return 1;
    }

    result = waveInAddBuffer(hWaveIn, &waveHdr, sizeof(WAVEHDR));
    if (result) {
        printf("Error adding wave input buffer: %u\n", result);
        return 1;
    }

    result = waveInStart(hWaveIn);
    if (result) {
        printf("Error starting wave input: %u\n", result);
        return 1;
    }
    printf("�^���J�n\n");


    // �^�������܂őҋ@
    Sleep(duration * 1000);

    printf("�^���C��\n");
    // �^����~
    result = waveInStop(hWaveIn);
    if (result) {
        printf("Error stopping wave input: %u\n", result);
        return 1;
    }

    result = waveInUnprepareHeader(hWaveIn, &waveHdr, sizeof(WAVEHDR));
    if (result) {
        printf("Error unpreparing wave input header: %u\n", result);
        return 1;
    }

    // �^���I��
    waveInClose(hWaveIn);
 

    // �^�������f�[�^��.wav�t�@�C���ɕۑ�
    const char* outputFileName = "recorded_sound.wav";
    saveToWavFile(outputFileName, buffer, bufferSize, sampleRate, channels, bitsPerSample);


    // �Đ�����
    HWAVEOUT hWaveOut;
    result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    if (result) {
        printf("Error opening wave output device: %u\n", result);
        return 1;
    }

    // �Đ��J�n
    result = waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
    if (result) {
        printf("Error preparing wave output header: %u\n", result);
        return 1;
    }

    result = waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));
    if (result) {
        printf("Error writing wave output buffer: %u\n", result);
        return 1;
    }

    // �Đ������܂őҋ@
    Sleep(duration * 1000);

    // �Đ���~
    result = waveOutReset(hWaveOut);
    if (result) {
        printf("Error resetting wave output: %u\n", result);
        return 1;
    }

    result = waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
    if (result) {
        printf("Error unpreparing wave output header: %u\n", result);
        return 1;
    }

    // �Đ��I��
    waveOutClose(hWaveOut);

    // ���������
    delete[] buffer;

    return 0;
}