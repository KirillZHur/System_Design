# Event Catalog

## CourseCreateRequested

### Тип сообщения

Command event

### Описание

Сообщение создаётся при получении запроса на создание курса. Оно не означает, что курс уже сохранён в базе данных, а только фиксирует намерение создать курс.

### Topic

`lms.course.commands`

### Producer

Course Command Service

### Consumers

- Course Consumer Service

### Payload

```json
{
  "event_id": "uuid",
  "event_type": "CourseCreateRequested",
  "occurred_at": "2026-05-12T12:00:00Z",
  "payload": {
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```

#### Назначение полей

| Поле                  | Тип     | Описание                           |
| --------------------- | ------- | ---------------------------------- |
| `event_id`            | string  | Уникальный идентификатор сообщения |
| `event_type`          | string  | Тип сообщения                      |
| `occurred_at`         | string  | Дата и время создания сообщения    |
| `payload.title`       | string  | Название курса                     |
| `payload.description` | string  | Описание курса                     |
| `payload.teacher_id`  | integer | Идентификатор преподавателя        |


### Delivery guarantee

`at-least-once`

### Message format

JSON

### Kafka key

В качестве ключа сообщения используется `event_id`.

### Повторная обработка

Consumer должен проверять, был ли уже обработан `event_id`. Если сообщение уже обработано, повторная запись курса не выполняется.

### CQRS role

`CourseCreateRequested` относится к command model.

Сообщение создаётся командным сервисом и используется для асинхронной синхронизации command model и read model.

### Write side

Course Command Service

### Read side updater

Course Consumer Service

### Read model

- PostgreSQL
- MongoDB
- Redis

### Implementation

Сообщение реализуется в рамках ЛР 6.

### Producer implementation

`Course Command Service`

Технологии:

- C++;
- userver;
- `kafka::ProducerComponent`;
- Redis.

### Consumer implementation

`Course Consumer Service`

Технологии:

- C++;
- userver;
- `kafka::ConsumerComponent`;
- PostgreSQL;
- MongoDB.

### Processing result

После успешной обработки сообщения:

- курс сохраняется в PostgreSQL;
- документ курса сохраняется в MongoDB;
- Kafka offset фиксируется consumer-сервисом.

### Docker

Producer и consumer запускаются как отдельные контейнеры через `docker compose up --build`.

## CourseCreated

### Тип сообщения

Domain event

### Описание

Событие фиксирует факт успешного создания курса после обработки команды consumer-сервисом.

### Topic

`lms.course.events`

### Producer

- Course Consumer Service

### Consumers
- Course Service
- Lesson Service
- Enrollment Service

### Payload
```json
{
  "event_id": "uuid",
  "event_type": "CourseCreated",
  "occurred_at": "2026-05-12T12:00:05Z",
  "payload": {
    "course_id": 1,
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```

#### Назначение полей
| Поле                  | Тип     | Описание                         |
| --------------------- | ------- | -------------------------------- |
| `event_id`            | string  | Уникальный идентификатор события |
| `event_type`          | string  | Тип события                      |
| `occurred_at`         | string  | Дата и время создания события    |
| `payload.course_id`   | integer | Идентификатор созданного курса   |
| `payload.title`       | string  | Название курса                   |
| `payload.description` | string  | Описание курса                   |
| `payload.teacher_id`  | integer | Идентификатор преподавателя      |

### Delivery guarantee

`at-least-once`

### Message format

JSON

### Kafka key

В качестве ключа сообщения используется `course_id`.

### Повторная обработка

Подписчики должны обрабатывать событие идемпотентно, так как при гарантии `at-least-once` событие может быть доставлено повторно.

### CQRS role

`CourseCreated` относится к domain events.

Событие может использоваться другими сервисами после того, как курс был успешно сохранён в read model.

### Read model updated

Да. После обработки команды данные курса становятся доступны для query-операций.

### Implementation status

Событие `CourseCreated` описано как архитектурное расширение. В обязательной реализации ЛР 6 основным сообщением является `CourseCreateRequested`, так как именно оно обеспечивает CQRS-сценарий создания курса через Kafka.