#define _CRT_SECURE_NO_WARNINGS // fopen関数の非推奨警告を抑制する
#include <Windows.h>
#include <string> // std::stringのために<string>ヘッダーファイルをインクルード
#include <cstdio> // fopen, fwrite, fcloseを使用するために<cstdio>をインクルード
#include <cmath>
#include <vector>
#include <iostream>
#pragma comment(lib, "Winmm.lib") // Winmm.libをリンクする指示

const double PI = 3.14159265358979323846;

const int SAMPLE_RATE = 44100;  // サンプルレート（Hz）
const int BITS_PER_SAMPLE = 16; // サンプルあたりのビット数
const int CHANNELS = 1;         // チャンネル数（モノラル）
const int RECORD_DURATION = 5; // 録音時間（秒）

// 録音したデータをWAVファイルに保存する
void saveToWavFile2(const std::vector<short>& buffer, const std::string& filename) {
    FILE* file = nullptr;
    if (fopen_s(&file, filename.c_str(), "wb") == 0 && file != nullptr) {
        // WAVファイルのヘッダー情報を設定
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

        // 音声データを書き込む
        fwrite(buffer.data(), sizeof(short), buffer.size(), file);

        fclose(file);
        std::cout << "ファイル保存" << std::endl;
    }
}

// .wavファイルにデータを保存する関数
void saveToWavFile(const char* outputFileName, char* buffer, DWORD bufferSize, int sampleRate, int channels, int bitsPerSample) {
    // .wavファイルヘッダの情報を設定
    WAVEFORMATEX wavHeader;
    memset(&wavHeader, 0, sizeof(WAVEFORMATEX));
    wavHeader.wFormatTag = WAVE_FORMAT_PCM;
    wavHeader.nChannels = channels;
    wavHeader.nSamplesPerSec = sampleRate;
    wavHeader.nAvgBytesPerSec = sampleRate * channels * bitsPerSample / 8;
    wavHeader.nBlockAlign = channels * bitsPerSample / 8;
    wavHeader.wBitsPerSample = bitsPerSample;
    wavHeader.cbSize = 0;

    // .wavファイルを作成して書き込む
    FILE* file = fopen(outputFileName, "wb");
    if (file == nullptr) {
        printf("Error opening output file.\n");
        return;
    }

    // .wavファイルヘッダを書き込む
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

    // 録音したデータをファイルに書き込む
    fwrite(buffer, 1, bufferSize, file);

    // ファイルを閉じる
    fclose(file);
    std::cout << "WAVファイル保存" << std::endl;
}


// サイン波を生成する関数
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
    // 設定
    const int duration = 10; // 録音・再生時間 (秒)
    const int sampleRate = 44100; // サンプルレート (Hz)
    const int bitsPerSample = 16; // 量子化ビット数 (16ビット)
    const int channels = 1; // モノラル

    // 録音準備
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

    // 録音バッファの準備
    const int bufferSize = duration * sampleRate * wfx.nBlockAlign;
    char* buffer = new char[bufferSize];
    WAVEHDR waveHdr;
    waveHdr.lpData = buffer;
    waveHdr.dwBufferLength = bufferSize;
    waveHdr.dwBytesRecorded = 0;
    waveHdr.dwUser = 0;
    waveHdr.dwFlags = 0;
    waveHdr.dwLoops = 0;

    // 録音開始
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
    printf("録音開始\n");


    // 録音完了まで待機
    Sleep(duration * 1000);

    printf("録音修了\n");
    // 録音停止
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

    // 録音終了
    waveInClose(hWaveIn);
 

    // 録音したデータを.wavファイルに保存
    const char* outputFileName = "recorded_sound.wav";
    saveToWavFile(outputFileName, buffer, bufferSize, sampleRate, channels, bitsPerSample);


    // 再生準備
    HWAVEOUT hWaveOut;
    result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    if (result) {
        printf("Error opening wave output device: %u\n", result);
        return 1;
    }

    // 再生開始
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

    // 再生完了まで待機
    Sleep(duration * 1000);

    // 再生停止
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

    // 再生終了
    waveOutClose(hWaveOut);

    // メモリ解放
    delete[] buffer;

    return 0;
}