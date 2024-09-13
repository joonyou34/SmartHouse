#include <Adafruit_NeoPixel.h>

class gameObject;

float SPF = 1/60.f;
unsigned long prevTime = 0;
float dt;
float g = 150.f;

void changeFPS(int fps) {
  SPF = 1.f/fps;
}

template <typename T>
T clamp(T value, T minValue, T maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

struct Color {
  byte r, g, b;
  Color(byte r = 0, byte g = 0, byte b = 0) : r(r), g(g), b(b) {}
};

template<typename T>
struct Box {
  T x0, y0, xf, yf;

  Box(T x0 = 0, T y0 = 0, T xf = 0, T yf = 0) :
  x0(x0), y0(y0), xf(xf), yf(yf) {}

  bool intersects(Box<T>& other) {
    return !(xf+1 < other.x0 || other.xf+1 < x0 || yf+1 < other.y0 || other.yf+1 < y0);
  }
};

class Sprite {
  private:
  void deleteDynamic() {
    if(img != nullptr) {
      delete[] img;
      for(int i = 0; i < h; i++)
        delete[] img[i];
    }

    if(alpha != nullptr) {
      for(int i = 0; i < h; i++)
        delete[] alpha[i];
      delete[] alpha;
    }
  }

  public:
  byte w, h;
  Color** img;
  bool**  alpha;
  byte oX, oY;
  
  Sprite(byte w = 0, byte h = 0, byte oX = 0, byte oY = 0, Color** img = nullptr, bool** alpha = nullptr) : 
  w(w), h(h), img(img), alpha(alpha), oX(oX), oY(oY)
  {
    if(img == nullptr) {
      img = new Color*[h];
      for (int i = 0; i < h; i++)
        img[i] = new Color[w];
    }

    if(alpha == nullptr) {
      alpha = new bool*[h];
      for (int i = 0; i < h; i++)
        alpha[i] = new bool[w];
    }
  }

  template <byte Rows, byte Cols>
  Sprite(Color (&spritePic)[Rows][Cols], bool (&spriteAlpha)[Rows][Cols], byte oX = 0, byte oY = 0) :
  w(Cols), h(Rows), oX(oX), oY(oY)
  {
    img = new Color*[h];
    for (int i = 0; i < h; i++)
      img[i] = new Color[w];

    alpha = new bool*[h];
    for (int i = 0; i < h; i++)
      alpha[i] = new bool[w];

    for(int i = 0; i < h; i++) {
      memcpy(img[i], spritePic[i], w * sizeof(Color));
      memcpy(alpha[i], spriteAlpha[i], w * sizeof(bool));
    }
  }

  Sprite(const Sprite& other) : 
    h(other.h), w(other.w), oX(other.oX), oY(other.oY)
  {
    img = new Color*[h];
    for (int i = 0; i < h; i++)
      img[i] = new Color[w];

    alpha = new bool*[h];
    for (int i = 0; i < h; i++)
      alpha[i] = new bool[w];
    
    for(int i = 0; i < h; i++) {
      memcpy(img[i], other.img[i], w * sizeof(Color));
      memcpy(alpha[i], other.alpha[i], w * sizeof(bool));
    }
  }

  void changeOrigin(byte oX, byte oY) {
    this->oX = oX;
    this->oY = oY;
  }


  ~Sprite() {
    deleteDynamic();
  }
};

class Window {
  private:
    const byte h, w, pixelCt, PIN, maxEntities;
    Adafruit_NeoPixel pixels;
    Color* gameScreen;
    bool* alpha;
    gameObject** renderStack;
    byte stkPtr = 0;

    inline byte cordToNum(byte x, byte y) {
      return x * w + y;
    }

  public:
    Window(byte h, byte w, byte PIN, byte maxEntities = 64) :
      h(h), w(w), pixelCt(h * w), PIN(PIN), maxEntities(maxEntities)
    {
      pixels = Adafruit_NeoPixel(pixelCt, PIN, NEO_GRB + NEO_KHZ800);
      gameScreen = new Color[pixelCt];
      alpha = new bool[pixelCt];
      renderStack = new gameObject*[maxEntities];
      pixels.begin();
    }

    void clear() {
      memset(alpha, 0, pixelCt);
    }

    void writePixel(byte x, byte y, Color c) {
      byte pixelNum = cordToNum(x, y);
      alpha[pixelNum] = true;
      gameScreen[pixelNum] = c;
    }

    void render();

    void pushObject(gameObject* obj) {
      if (stkPtr == maxEntities)
        return;

      renderStack[stkPtr++] = obj;
    }

    ~Window() {
      delete[] gameScreen;
      delete[] alpha;
      delete[] renderStack;
    }
};


class gameObject {
  public:
    Sprite sprite;
    Box<float> hitbox;
    float x, y;
    float vx = 0, vy = 0;

    gameObject(Box<float> hitbox = Box<float>(), Sprite sprite = Sprite(), float x = 0.f, float y = 0.f)
      : x(x), y(y), hitbox(hitbox), sprite(sprite) {}


    void render(Window& targetWindow) {
      targetWindow.pushObject(this);
    }

    void update() {
      x += vx*dt;
      y += vy*dt;
    }

    Box<float> getAbsoluteHitbox() {
      return Box<float>(
        hitbox.x0 + x - sprite.oX,
        hitbox.y0 + y - sprite.oY,
        hitbox.xf + x - sprite.oX,
        hitbox.yf + y - sprite.oY
      );
    }

    bool intersects(Box<float>& other) {
      return getAbsoluteHitbox().intersects(other);
    }

    bool intersects(gameObject& other) {
       
       Box<float> tmp = other.getAbsoluteHitbox();

      return intersects(tmp);
    }
};




void Window::render() {
  while (stkPtr--) {
    gameObject& obj = *renderStack[stkPtr];
    int objX = round(obj.x), objY = round(obj.y);
    for (int i = max(obj.sprite.oX-objX, 0); i < obj.sprite.h; i++) {
      byte xcord = objX + i - obj.sprite.oX;
      if (xcord >= h) break;
      for (int j = max(obj.sprite.oY-objY, 0); j < obj.sprite.w; j++) {
        if (!obj.sprite.alpha[i][j]) continue;

        byte ycord = objY + j - obj.sprite.oY;
        if (ycord >= w) break;

        byte pixelNum = cordToNum(xcord, ycord);
        if (alpha[pixelNum]) continue;


        alpha[pixelNum] = true;
        gameScreen[pixelNum] = obj.sprite.img[i][j];
      }
    }
  }
  stkPtr = 0;

  for (byte i = 0; i < pixelCt; ++i) {
    if (alpha[i])
      pixels.setPixelColor(i, pixels.Color(gameScreen[i].r, gameScreen[i].g, gameScreen[i].b));
    else
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }

  pixels.show();
}

void calcDT() {
  dt = (millis() - prevTime)/1000.f;
  prevTime = millis();
}