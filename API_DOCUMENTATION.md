# Документация API модуля FractalCORE

## Оглавление
1. [Введение](#введение)
2. [Основные управление](#основные-управление)
3. [ECS система](#ecs-система)
4. [Системы и параллельная обработка](#системы-и-параллельная-обработка)
5. [События и обмен сообщениями](#события-и-обмен-сообщениями)
6. [SQLite хранилище](#sqlite-хранилище)
7. [Примеры использования](#примеры-использования)

---

## Введение

**ModuleAPI** — это высокоуровневая обёртка C++ для взаимодействия с FractalCORE Gateway. Класс обеспечивает безопасную работу с Entity Component System (ECS), системой событий и параллельной обработкой данных.

### Инициализация

```cpp
ModuleAPI api(gateway_pointer);
```

Конструктор принимает указатель на `FractalCORE_Gateway`. Если указатель равен `nullptr`, будет выброшено исключение `std::invalid_argument`.

---

## Основные управление

### `Entity createEntity()`

Создаёт новую сущность в системе.

**Возвращаемое значение:** `Entity` — структура с уникальным ID сущности.

**Пример:**
```cpp
Entity player = api.createEntity();
std::cout << "Player ID: " << player.id << std::endl;
```

---

### `void enqueueTask(const Task& task)`

Ставит задачу в очередь для выполнения.

**Параметры:**
- `task` — структура `Task` содержащая функцию обратного вызова и флаг `isBackTask`

**Пример:**
```cpp
Task myTask;
myTask.func = [] { std::cout << "Task executed!" << std::endl; };
myTask.isBackTask = true;
api.enqueueTask(myTask);
```

---

### `void registerIntervalTask(const TickTask& tickTask)`

Регистрирует периодическую задачу, которая выполняется с заданным интервалом.

**Параметры:**
- `tickTask` — структура `TickTask` содержащая:
  - `func` — функция для выполнения
  - `intervalMs` — интервал в миллисекундах
  - `executionsRemaining` — количество оставшихся выполнений
  - `active` — флаг активности

**Пример:**
```cpp
TickTask intervalTask;
intervalTask.func = [] { std::cout << "Tick!" << std::endl; };
intervalTask.intervalMs = std::chrono::milliseconds(1000);
intervalTask.executionsRemaining = 5;
intervalTask.active = true;
api.registerIntervalTask(intervalTask);
```

---

### `float getDeltaTime() const`

Получает время, прошедшее с последнего кадра (в секундах).

**Возвращаемое значение:** `float` — дельта время.

**Пример:**
```cpp
float dt = api.getDeltaTime();
position += velocity * dt;
```

---

### `Clock& getEngineClock()`

Получает объект часов двигателя.

**Возвращаемое значение:** Ссылка на объект `Clock`.

**Исключения:** `std::runtime_error` если часы недоступны.

**Пример:**
```cpp
Clock& clock = api.getEngineClock();
// Использование часов...
```

---

### `void stop()`

Останавливает работу двигателя.

**Пример:**
```cpp
api.stop();
```

---

## ECS система

### `template<typename T> void registerComponent(const std::string& name, size_t capacity = 10000)`

Регистрирует новый тип компонента в системе.

**Параметры:**
- `name` — уникальное имя компонента
- `capacity` — максимальное количество экземпляров компонента (по умолчанию 10000)

**Шаблонный параметр:** `T` — тип компонента

**Пример:**
```cpp
struct Position {
    float x, y, z;
};

api.registerComponent<Position>("Position", 5000);
```

---

### `template<typename T> void attachComponent(Entity e, const std::string& name, const T& data)`

Присоединяет компонент к сущности.

**Параметры:**
- `e` — сущность, к которой присоединяется компонент
- `name` — имя компонента
- `data` — данные компонента

**Шаблонный параметр:** `T` — тип компонента

**Пример:**
```cpp
Position pos{10.0f, 20.0f, 0.0f};
api.attachComponent<Position>(player, "Position", pos);
```

---

### `void removeComponent(Entity e, const std::string& name)`

Удаляет компонент из сущности.

**Параметры:**
- `e` — сущность
- `name` — имя компонента

**Пример:**
```cpp
api.removeComponent(player, "Position");
```

---

### `template<typename T> T* getComponent(Entity e, const std::string& name)`

Получает указатель на компонент сущности.

**Параметры:**
- `e` — сущность
- `name` — имя компонента

**Возвращаемое значение:** Указатель на компонент или `nullptr` если компонент не найден.

**Шаблонный параметр:** `T` — тип компонента

**Пример:**
```cpp
Position* pos = api.getComponent<Position>(player, "Position");
if (pos) {
    pos->x += 5.0f;
}
```

---

### `bool hasComponent(Entity e, const std::string& name)`

Проверяет наличие компонента у сущности.

**Параметры:**
- `e` — сущность
- `name` — имя компонента

**Возвращаемое значение:** `true` если компонент присутствует, `false` иначе.

**Пример:**
```cpp
if (api.hasComponent(player, "Position")) {
    std::cout << "Player has Position component" << std::endl;
}
```

---

### `void registerGroup(const std::vector<std::string>& componentNames)`

Регистрирует группу компонентов для оптимизированной параллельной обработки.

**Параметры:**
- `componentNames` — вектор имён компонентов в группе

**Пример:**
```cpp
api.registerGroup({"Position", "Velocity", "Transform"});
```

---

## Системы и параллельная обработка

### `template<typename T> void registerSystem(const std::string& componentName, void (*updateFunc)(Entity, T&, float), TriggerType trigger = TriggerType::Always, float timeInterval = 0.0f, size_t tickInterval = 0)`

Регистрирует систему ECS для обработки компонентов определённого типа.

**Параметры:**
- `componentName` — имя компонента, который обрабатывает система
- `updateFunc` — функция обновления с сигнатурой `void(Entity, T&, float)`
  - `Entity` — сущность
  - `T&` — компонент
  - `float` — дельта время
- `trigger` — тип триггера (`Always`, `TimeInterval`, `TickInterval`)
- `timeInterval` — интервал времени в секундах (для `TimeInterval`)
- `tickInterval` — интервал тиков (для `TickInterval`)

**Пример:**
```cpp
api.registerSystem<Position>(
    "Position",
    [](Entity e, Position& pos, float dt) {
        pos.x += 1.0f * dt;
    },
    TriggerType::Always
);
```

---

### `template<typename T> void updateParallel(const std::string& componentName, void (*func)(Entity, T&), size_t chunkSize = 64)`

Обновляет все компоненты определённого типа параллельно.

**Параметры:**
- `componentName` — имя компонента
- `func` — функция обработки с сигнатурой `void(Entity, T&)`
- `chunkSize` — размер блока данных для параллельной обработки (по умолчанию 64)

**Пример:**
```cpp
api.updateParallel<Position>(
    "Position",
    [](Entity e, Position& pos) {
        pos.x += 1.0f;
    },
    128
);
```

---

### `void updateParallelGroup(const std::vector<std::string>& componentNames, void (*updateFunc)(size_t start, size_t end, void* userCtx), void* userCtx = nullptr, size_t chunkSize = 1024)`

Обновляет группу компонентов параллельно с использованием диапазонов индексов.

**Параметры:**
- `componentNames` — вектор имён компонентов в группе
- `updateFunc` — функция обработки с сигнатурой `void(size_t start, size_t end, void* userCtx)`
- `userCtx` — пользовательский контекст
- `chunkSize` — размер блока для параллельной обработки (по умолчанию 1024)

**Пример:**
```cpp
std::vector<std::string> components = {"Position", "Velocity"};
api.updateParallelGroup(
    components,
    [](size_t start, size_t end, void* ctx) {
        for (size_t i = start; i < end; ++i) {
            // Обработка блока данных
        }
    }
);
```

---

## События и обмен сообщениями

### `template<typename T> void subscribe(const std::string& eventName, void (*handler)(const T&, void*), void* userData = nullptr)`

Подписывается на событие определённого типа.

**Параметры:**
- `eventName` — имя события
- `handler` — функция обработчика с сигнатурой `void(const T&, void*)`
  - `const T&` — данные события
  - `void*` — пользовательские данные
- `userData` — дополнительные данные пользователя

**Пример:**
```cpp
struct PlayerDamagedEvent {
    uint32_t playerId;
    float damage;
};

api.subscribe<PlayerDamagedEvent>(
    "player_damaged",
    [](const PlayerDamagedEvent& event, void* ctx) {
        std::cout << "Player " << event.playerId 
                  << " took " << event.damage << " damage" << std::endl;
    }
);
```

---

### `template<typename T> void emitEvent(const std::string& eventName, const T& data)`

Немедленно отправляет событие всем подписчикам.

**Параметры:**
- `eventName` — имя события
- `data` — данные события

**Пример:**
```cpp
PlayerDamagedEvent event{player.id, 10.0f};
api.emitEvent<PlayerDamagedEvent>("player_damaged", event);
```

---

### `template<typename T> void pushEvent(const std::string& eventName, const T& data)`

Добавляет событие в очередь для отправки в следующем цикле.

**Параметры:**
- `eventName` — имя события
- `data` — данные события

**Пример:**
```cpp
PlayerDamagedEvent event{player.id, 10.0f};
api.pushEvent<PlayerDamagedEvent>("player_damaged", event);
```

---

## SQLite хранилище

### `bool setSQLString(const char* key, const char* value) noexcept`

Сохраняет строковое значение в SQLite базу данных.

**Параметры:**
- `key` — ключ
- `value` — значение

**Возвращаемое значение:** `true` если успешно, `false` иначе.

**Пример:**
```cpp
if (api.setSQLString("player_name", "Hero")) {
    std::cout << "Saved player name" << std::endl;
}
```

---

### `size_t getSQLString(const char* key, char* outBuffer, size_t bufferSize) noexcept`

Получает строковое значение из SQLite базы данных.

**Параметры:**
- `key` — ключ
- `outBuffer` — буфер для вывода
- `bufferSize` — размер буфера

**Возвращаемое значение:** Количество скопированных байт или 0 если ключ не найден.

**Пример:**
```cpp
char buffer[256];
size_t len = api.getSQLString("player_name", buffer, sizeof(buffer));
if (len > 0) {
    std::cout << "Player name: " << buffer << std::endl;
}
```

---

### `bool SQLExist(const char* key) noexcept`

Проверяет наличие ключа в SQLite базе данных.

**Параметры:**
- `key` — ключ

**Возвращаемое значение:** `true` если ключ существует, `false` иначе.

**Пример:**
```cpp
if (api.SQLExist("player_name")) {
    std::cout << "Player name exists in database" << std::endl;
}
```

---

### `bool SQLExecute(const char* sql) noexcept`

Выполняет SQL запрос.

**Параметры:**
- `sql` — SQL команда

**Возвращаемое значение:** `true` если успешно, `false` иначе.

**Пример:**
```cpp
api.SQLExecute("CREATE TABLE IF NOT EXISTS players (id INTEGER, name TEXT)");
api.SQLExecute("INSERT INTO players VALUES (1, 'Hero')");
```

---

## Примеры использования

### Пример 1: Создание и управление сущностью

```cpp
// Создание сущности
Entity player = api.createEntity();

// Регистрация компонентов
struct Health { float hp; float maxHp; };
struct Position { float x, y, z; };

api.registerComponent<Health>("Health", 1000);
api.registerComponent<Position>("Position", 10000);

// Присоединение компонентов
api.attachComponent<Health>(player, "Health", {100.0f, 100.0f});
api.attachComponent<Position>(player, "Position", {0.0f, 0.0f, 0.0f});

// Получение компонента
if (Health* health = api.getComponent<Health>(player, "Health")) {
    health->hp -= 10.0f;
}
```

---

### Пример 2: Система обновления позиций

```cpp
struct Velocity { float x, y, z; };

api.registerComponent<Velocity>("Velocity", 10000);

// Регистрация системы
api.registerSystem<Position>(
    "Position",
    [&api](Entity e, Position& pos, float dt) {
        Velocity* vel = api.getComponent<Velocity>(e, "Velocity");
        if (vel) {
            pos.x += vel->x * dt;
            pos.y += vel->y * dt;
            pos.z += vel->z * dt;
        }
    },
    TriggerType::Always
);
```

---

### Пример 3: Параллельная обработка

```cpp
api.updateParallel<Position>(
    "Position",
    [](Entity e, Position& pos) {
        // Применить гравитацию
        pos.y -= 9.8f * api.getDeltaTime();
    },
    256
);
```

---

### Пример 4: События

```cpp
struct HealthChangedEvent {
    Entity entity;
    float oldHealth;
    float newHealth;
};

// Подписка на событие
api.subscribe<HealthChangedEvent>(
    "health_changed",
    [](const HealthChangedEvent& event, void* ctx) {
        if (event.newHealth <= 0.0f) {
            std::cout << "Entity died!" << std::endl;
        }
    }
);

// Отправка события
Health* health = api.getComponent<Health>(player, "Health");
if (health) {
    float oldHp = health->hp;
    health->hp -= 50.0f;
    
    HealthChangedEvent event{player, oldHp, health->hp};
    api.emitEvent<HealthChangedEvent>("health_changed", event);
}
```

---

### Пример 5: Сохранение и загрузка данных

```cpp
// Сохранение
api.setSQLString("last_level", "level_5");
api.setSQLString("player_score", "9850");

// Загрузка
char level[64];
api.getSQLString("last_level", level, sizeof(level));
std::cout << "Last level: " << level << std::endl;

// Проверка существования
if (api.SQLExist("player_score")) {
    char score[32];
    api.getSQLString("player_score", score, sizeof(score));
    std::cout << "Score: " << score << std::endl;
}
```

---

## Типы данных

### `enum class TriggerType`
- `Always` — система выполняется каждый кадр
- `TimeInterval` — система выполняется с интервалом времени
- `TickInterval` — система выполняется с интервалом тиков

### `struct Entity`
```cpp
struct Entity {
    uint32_t id;  // Уникальный идентификатор сущности
};
```

### `struct Task`
```cpp
struct Task {
    std::function<void()> func;  // Функция для выполнения
    bool isBackTask = true;      // Флаг фоновой задачи
};
```

### `struct TickTask`
```cpp
struct TickTask {
    std::function<void()> func;
    std::chrono::milliseconds intervalMs;
    std::chrono::steady_clock::time_point lastExecutedMs;
    size_t id;
    size_t executionsRemaining;
    bool active;
    float timeAccumulator = 0.0f;
    bool isBackTask = true;
};
```

---

## Обработка ошибок

Большинство методов используют исключения при критических ошибках:
- `std::invalid_argument` — неверные аргументы
- `std::runtime_error` — ошибки во время выполнения

Методы SQLite и некоторые операции используют `noexcept` и возвращают статусы успеха/неудачи.

---

## Советы по производительности

1. **Кэширование Event ID:** Класс автоматически кэширует ID событий после первой регистрации.

2. **Параллельная обработка:** Используйте `updateParallel` вместо последовательной обработки для больших наборов данных.

3. **Группы компонентов:** Используйте `registerGroup` для оптимизированной обработки нескольких связанных компонентов.

4. **Размер блока:** Экспериментируйте с параметром `chunkSize` для оптимальной производительности на вашей платформе.

---

## Версия документации
- **Версия:** 1.0
- **Дата:** Январь 2026
- **Язык:** Русский
