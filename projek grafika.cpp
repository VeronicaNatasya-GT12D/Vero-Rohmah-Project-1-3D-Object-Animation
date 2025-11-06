/*
 * proyek.cpp - Proyek Grafika Komputer
 * Scene lab komputer 3D dengan kontrol keyboard dan rotasi kursi.
 * (Versi ini MENGHILANGKAN bayangan proyektif).
 *
 * Kontrol:
 * - Panah Atas/Bawah: Gerakkan kursi maju/mundur
 * - Panah Kiri/Kanan: Gerakkan kursi ke kiri/kanan (batas berubah)
 * - 'A' (Kapital): Redupkan layar monitor
 * - 'a' (Kecil): Terangkan layar monitor
 * - 'L' (Kapital): Ubah posisi sumber cahaya
 * - 'D' (Kapital): Putar kursi ke kanan (hanya jika mundur)
 * - 'd' (Kecil): Putar kursi ke kiri (hanya jika mundur)
 *
 * Untuk meng-compile di Linux/macOS (dengan freeglut):
 * g++ proyek.cpp -o proyek -lGL -lGLU -lglut
 */

#include <GL/glut.h>
#include <cmath>

 // ===================================================================
 // Variabel Global untuk Kontrol & Animasi
 // ===================================================================

float chairZPos = 4.0f;
float chairXPos = 0.0f;
float chairRotationY = 0.0f; // Untuk rotasi kursi
float monitorBrightness = 1.0f;

// Batasan Gerak Kursi
const float CHAIR_Z_FORWARD_LIMIT = 3.5f;
const float CHAIR_Z_BACKWARD_LIMIT = 8.0f;
// BATAS SEMPIT (DI KOLONG MEJA)
const float CHAIR_X_NARROW_LEFT_LIMIT = -2.5f;
const float CHAIR_X_NARROW_RIGHT_LIMIT = 2.5f;
// BATAS LEBAR (DI LUAR KOLONG MEJA)
const float CHAIR_X_WIDE_LEFT_LIMIT = -8.0f; // Batas tembok
const float CHAIR_X_WIDE_RIGHT_LIMIT = 8.0f; // Batas tembok

const float CHAIR_Z_SAFE_SIDE_MOVE = 3.6f; // Titik Z untuk ganti batas
const float CHAIR_MOVE_SPEED = 0.1f;
const float CHAIR_ROTATE_SPEED = 5.0f;
const float MONITOR_BRIGHTNESS_STEP = 0.1f;

// Variabel untuk posisi cahaya
float lightX = 10.0f;
float lightY = 15.0f;
float lightZ = 10.0f;
int lightPositionIndex = 0;

// Posisi lantai
const float FLOOR_Y_POSITION = -4.0f;
const float FLOOR_Y_SURFACE = -3.75f; // Di atas lantai (0.5 tebal / 2)

// ===================================================================
// Fungsi-fungsi Helper untuk Menggambar Objek
// ===================================================================

/**
 * Fungsi untuk mengatur properti material sebuah objek.
 */
void setMaterial(float ambientR, float ambientG, float ambientB,
    float diffuseR, float diffuseG, float diffuseB,
    float specularR, float specularG, float specularB,
    float shininess) {
    GLfloat ambient[] = { ambientR, ambientG, ambientB, 1.0f };
    GLfloat diffuse[] = { diffuseR, diffuseG, diffuseB, 1.0f };
    GLfloat specular[] = { specularR, specularG, specularB, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

/**
 * Menggambar Meja (Permukaan + 4 Kaki)
 */
void drawDesk() {
    // Material kayu untuk meja (Shininess 10.0f)
    setMaterial(0.4f, 0.3f, 0.2f, 0.8f, 0.7f, 0.5f, 0.1f, 0.1f, 0.1f, 10.0f);

    // Permukaan Meja
    glPushMatrix();
    glScalef(7.0f, 0.5f, 5.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Kaki Meja (4 buah)
    // Kaki 1 (depan-kanan)
    glPushMatrix();
    glTranslatef(3.0f, -2.5f, 2.0f);
    glScalef(0.5f, 4.5f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Kaki 2 (depan-kiri)
    glPushMatrix();
    glTranslatef(-3.0f, -2.5f, 2.0f);
    glScalef(0.5f, 4.5f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Kaki 3 (belakang-kanan)
    glPushMatrix();
    glTranslatef(3.0f, -2.5f, -2.0f);
    glScalef(0.5f, 4.5f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Kaki 4 (belakang-kiri)
    glPushMatrix();
    glTranslatef(-3.0f, -2.5f, -2.0f);
    glScalef(0.5f, 4.5f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();
}

/**
 * Menggambar Monitor (Base, Tiang, Layar)
 */
void drawMonitor() {
    // Material monitor (plastik gelap, Shininess DITURUNKAN ke 20.0f)
    setMaterial(0.1f, 0.1f, 0.1f, 0.2f, 0.2f, 0.2f, 0.5f, 0.5f, 0.5f, 20.0f);

    // Base (Kaki Monitor)
    glPushMatrix();
    glScalef(2.0f, 0.2f, 1.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Tiang Monitor
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.0f);
    glScalef(0.3f, 1.5f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Frame Layar
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 0.0f);
    glScalef(3.5f, 2.5f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Layar (bagian 'nyala')
    // Material layar (emissive, tidak terpengaruh cahaya lain)
    GLfloat screen_emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default non-emissive
    if (monitorBrightness > 0) { // Jika layar tidak mati total
        screen_emissive[0] = 0.05f * monitorBrightness; // Sedikit emissive untuk kesan menyala
        screen_emissive[1] = 0.07f * monitorBrightness;
        screen_emissive[2] = 0.09f * monitorBrightness;
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, screen_emissive);

    // Warna diffuse dan ambient layar akan berubah berdasarkan brightness
    setMaterial(0.1f * monitorBrightness, 0.14f * monitorBrightness, 0.18f * monitorBrightness, // Ambient
        0.5f * monitorBrightness, 0.7f * monitorBrightness, 0.9f * monitorBrightness,   // Diffuse
        0.0f, 0.0f, 0.0f, // Layar tidak memantulkan specular
        0.0f);            // Shininess 0

    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 0.16f); // Maju sedikit dari frame
    glScalef(3.3f, 2.3f, 0.01f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Pastikan emissive kembali ke nol untuk objek selanjutnya
    GLfloat zero_emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero_emissive);
}

/**
 * Menggambar Keyboard (Sederhana)
 */
void drawKeyboard() {
    // Material keyboard (plastik gelap)
    setMaterial(0.05f, 0.05f, 0.05f,
        0.1f, 0.1f, 0.1f,
        0.3f, 0.3f, 0.3f,
        30.0f);

    glPushMatrix();
    glScalef(3.5f, 0.15f, 1.2f);
    glutSolidCube(1.0);
    glPopMatrix();
}

/**
 * Menggambar Kursi (Objek Kompleks)
 */
void drawChair() {
    // Material kursi (Shininess 40.0f)
    setMaterial(0.15f, 0.15f, 0.15f, 0.3f, 0.3f, 0.3f, 0.4f, 0.4f, 0.4f, 40.0f);

    // Dudukan
    glPushMatrix();
    glScalef(2.0f, 0.4f, 2.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Sandaran
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, -0.8f); // Pindah ke atas dan belakang dudukan
    glScalef(2.0f, 3.0f, 0.4f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Tiang tengah (metalik, Shininess DITURUNKAN ke 15.0f agar lebih lembut)
    setMaterial(0.2f, 0.2f, 0.2f,
        0.4f, 0.4f, 0.4f,
        0.8f, 0.8f, 0.8f, // Specular tinggi
        15.0f);           // Shininess DITURUNKAN
    glPushMatrix();
    glTranslatef(0.0f, -1.25f, 0.0f);
    glScalef(0.3f, 2.0f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Kaki-kaki (5 buah, metalik, Shininess DITURUNKAN ke 15.0f)
    for (int i = 0; i < 5; i++) {
        glPushMatrix();
        glRotatef(i * 72.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.75f, -2.25f, 0.0f);
        glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        glScalef(1.5f, 0.2f, 0.2f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

/**
 * Menggambar Lantai dan Tembok (Digabung)
 */
void drawRoom() {
    // Lantai
    setMaterial(0.1f, 0.1f, 0.1f, 0.4f, 0.4f, 0.4f, 0.05f, 0.05f, 0.05f, 5.0f);
    glPushMatrix();
    glTranslatef(0.0f, FLOOR_Y_POSITION, 0.0f); // Posisi lantai
    glScalef(20.0f, 0.5f, 20.0f);   // Ukuran lantai
    glutSolidCube(1.0);
    glPopMatrix();

    // Tembok Belakang
    setMaterial(0.1f, 0.1f, 0.15f,
        0.5f, 0.5f, 0.6f,
        0.05f, 0.05f, 0.05f,
        10.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -9.0f); // Posisi tembok belakang
    glScalef(20.0f, 10.0f, 0.5f);   // Ukuran tembok
    glutSolidCube(1.0);
    glPopMatrix();

    // Tembok Kanan
    glPushMatrix();
    glTranslatef(9.0f, 0.0f, 0.0f); // Posisi tembok kanan
    glScalef(0.5f, 10.0f, 20.0f);   // Ukuran tembok
    glutSolidCube(1.0);
    glPopMatrix();

    // Tembok Kiri
    glPushMatrix();
    glTranslatef(-9.0f, 0.0f, 0.0f); // Posisi tembok kiri
    glScalef(0.5f, 10.0f, 20.0f);   // Ukuran tembok
    glutSolidCube(1.0);
    glPopMatrix();
}


// ===================================================================
// FUNGSI BAYANGAN DIHAPUS
// ===================================================================


// ===================================================================
// Fungsi Callback Keyboard
// ===================================================================

/**
 * Fungsi Callback untuk Tombol Keyboard Biasa (ASCII)
 */
void keyboardPress(unsigned char key, int x, int y) {
    switch (key) {
    case 'a': // 'a' kecil (Menerangkan)
        monitorBrightness += MONITOR_BRIGHTNESS_STEP;
        if (monitorBrightness > 1.0f) {
            monitorBrightness = 1.0f; // Batas atas (paling terang)
        }
        break;
    case 'A': // 'A' besar (Meredupkan)
        monitorBrightness -= MONITOR_BRIGHTNESS_STEP;
        if (monitorBrightness < 0.0f) {
            monitorBrightness = 0.0f; // Batas bawah (paling gelap)
        }
        break;
    case 'l':
    case 'L': // 'L' atau 'l' (Mengubah posisi cahaya)
    { // Kurung kurawal untuk scope C++
        lightPositionIndex = (lightPositionIndex + 1) % 4; // 4 posisi berbeda

        switch (lightPositionIndex) {
        case 0: // Default (atas-kanan-depan)
            lightX = 10.0f; lightY = 15.0f; lightZ = 10.0f; break;
        case 1: // Atas-Kiri-Depan
            lightX = -10.0f; lightY = 15.0f; lightZ = 10.0f; break;
        case 2: // Atas-Belakang
            lightX = 0.0f; lightY = 15.0f; lightZ = -10.0f; break;
        case 3:
            lightX = 5.0f; lightY = 5.0f; lightZ = 15.0f; break;
        }
        // Update posisi cahaya di OpenGL
        GLfloat light_pos[] = { lightX, lightY, lightZ, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    }
    break;

    // Kontrol Rotasi Kursi
    case 'D': // 'D' Kapital (Putar Kanan)
        // Hanya bisa berputar jika sudah mundur
        if (chairZPos >= CHAIR_Z_SAFE_SIDE_MOVE) {
            chairRotationY += CHAIR_ROTATE_SPEED;
        }
        break;
    case 'd': // 'd' kecil (Putar Kiri)
        // Hanya bisa berputar jika sudah mundur
        if (chairZPos >= CHAIR_Z_SAFE_SIDE_MOVE) {
            chairRotationY -= CHAIR_ROTATE_SPEED;
        }
        break;
    }
    glutPostRedisplay(); // Minta gambar ulang scene
}

/**
 * Fungsi Callback untuk Tombol Keyboard Spesial (Panah)
 */
void specialKeyPress(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP: // Panah Atas (Maju)
        chairZPos -= CHAIR_MOVE_SPEED;
        // Cek tabrakan dengan meja
        if (chairZPos < CHAIR_Z_FORWARD_LIMIT) {
            chairZPos = CHAIR_Z_FORWARD_LIMIT;
        }
        break;
    case GLUT_KEY_DOWN: // Panah Bawah (Mundur)
        chairZPos += CHAIR_MOVE_SPEED;
        // Batas mundur (lebih jauh karena ada tembok)
        if (chairZPos > CHAIR_Z_BACKWARD_LIMIT) {
            chairZPos = CHAIR_Z_BACKWARD_LIMIT;
        }
        break;
    case GLUT_KEY_LEFT: // Panah Kiri
    { // Kurung kurawal diperlukan untuk deklarasi variabel lokal dalam switch
        // LOGIKA GERAK BARU: Tentukan batas berdasarkan posisi Z
        float currentLeftLimit = (chairZPos < CHAIR_Z_SAFE_SIDE_MOVE) ?
            CHAIR_X_NARROW_LEFT_LIMIT :
            CHAIR_X_WIDE_LEFT_LIMIT;

        chairXPos -= CHAIR_MOVE_SPEED;
        if (chairXPos < currentLeftLimit) {
            chairXPos = currentLeftLimit;
        }
    }
    break;
    case GLUT_KEY_RIGHT: // Panah Kanan
    { // Kurung kurawal
        // LOGIKA GERAK BARU: Tentukan batas berdasarkan posisi Z
        float currentRightLimit = (chairZPos < CHAIR_Z_SAFE_SIDE_MOVE) ?
            CHAIR_X_NARROW_RIGHT_LIMIT :
            CHAIR_X_WIDE_RIGHT_LIMIT;

        chairXPos += CHAIR_MOVE_SPEED;
        if (chairXPos > currentRightLimit) {
            chairXPos = currentRightLimit;
        }
    }
    break;
    }
    glutPostRedisplay(); // Minta gambar ulang scene
}


// ===================================================================
// Fungsi Utama GLUT
// ===================================================================

/**
 * Fungsi Inisialisasi OpenGL
 */
void init() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // KODE BAYANGAN DIHAPUS
    // glDisable(GL_POLYGON_OFFSET_FILL);
    // glPolygonOffset(-1.0f, -1.0f); 

    // Atur properti cahaya (DIBUAT LEBIH LEMBUT)
    GLfloat light_pos[] = { lightX, lightY, lightZ, 1.0f };
    // Ambient diturunkan sedikit, agar bayangan lebih gelap
    GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    // Diffuse diturunkan agar tidak "gonjreng"
    GLfloat diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    // Specular (kilau) diturunkan agar lebih lembut
    GLfloat specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

/**
 * Fungsi Reshape (Dipanggil saat jendela di-resize)
 */
void reshape(int w, int h) {
    if (h == 0) h = 1; // Mencegah pembagian dengan nol
    float ratio = 1.0 * w / h;

    glViewport(0, 0, w, h); // Set viewport ke seluruh jendela

    // Menerapkan "Proyeksi 3D" (Perspektif)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // (Field of View, Aspect Ratio, Near Clip, Far Clip)
    gluPerspective(45.0, ratio, 1.0, 100.0);

    // Kembali ke mode ModelView
    glMatrixMode(GL_MODELVIEW);
}

/**
 * Fungsi Display (Menggambar ulang scene)
 * URUTAN PENGGAMBARAN DIKEMBALIKAN
 */
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(8.0, 7.0, 15.0,  // Posisi kamera
        0.0, 0.0, 0.0,    // Lihat ke origin
        0.0, 1.0, 0.0);   // Sumbu Y adalah 'atas'

    // --- Menggambar Semua Objek ---

    // 1. Gambar Ruangan (Lantai & Tembok)
    drawRoom();

    // 2. Gambar Meja (Objek Asli)
    drawDesk();

    // 3. Gambar Monitor (Objek Asli)
    glPushMatrix();
    glTranslatef(0.0f, 0.25f + 0.8f, 1.5f);
    glScalef(0.8f, 0.8f, 0.8f);
    drawMonitor();
    glPopMatrix();

    // 4. Gambar Keyboard (Objek Asli)
    glPushMatrix();
    // Posisi Y: di atas permukaan meja + setengah ketebalan keyboard
    // Posisi Z: sedikit di depan monitor
    glTranslatef(0.0f, 0.25f + (0.15f / 2.0f) + 0.1f, 2.5f); // 0.1f untuk offset agar tidak menempel
    glScalef(1.0f, 1.0f, 0.8f);
    drawKeyboard();
    glPopMatrix();

    // 5. Gambar Kursi (Objek Asli, DIKONTROL)
    glPushMatrix();
    // PERBAIKAN: Posisi Y -1.95f agar menapak di lantai
    glTranslatef(chairXPos, -1.95f, chairZPos);
    // Terapkan rotasi DARI keyboard
    glRotatef(180.0f + chairRotationY, 0.0f, 1.0f, 0.0f);
    glScalef(0.8f, 0.8f, 0.8f);
    drawChair();
    glPopMatrix();

    glutSwapBuffers();
}


/**
 * Fungsi Main
 */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("Proyek Grafika - Kontrol Keyboard (Rotasi)");

    init(); // Panggil inisialisasi OpenGL

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardPress);
    glutSpecialFunc(specialKeyPress);

    glutMainLoop();
    return 0;
}