db = db.getSiblingDB('lms_mongo_db');

const collections = db.getCollectionNames();

if (!collections.includes("courses")) {
  db.createCollection("courses");
}

db.runCommand({
  collMod: "courses",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["_id", "title", "teacher_id", "created_at", "lessons"],
      properties: {
        _id: {
          bsonType: ["int", "long"],
          description: "Идентификатор курса должен быть целым числом"
        },
        title: {
          bsonType: "string",
          minLength: 2,
          description: "Название курса должно быть строкой длиной не менее 2 символов"
        },
        description: {
          bsonType: ["string", "null"],
          description: "Описание курса должно быть строкой или null"
        },
        teacher_id: {
          bsonType: ["int", "long"],
          description: "Идентификатор преподавателя должен быть целым числом"
        },
        created_at: {
          bsonType: "date",
          description: "Дата создания курса должна иметь тип date"
        },
        lessons: {
          bsonType: "array",
          description: "Поле lessons должно быть массивом",
          items: {
            bsonType: "object",
            required: ["id", "title", "position", "created_at"],
            properties: {
              id: {
                bsonType: ["int", "long"],
                description: "Идентификатор урока должен быть целым числом"
              },
              title: {
                bsonType: "string",
                minLength: 2,
                description: "Название урока должно быть строкой длиной не менее 2 символов"
              },
              content: {
                bsonType: ["string", "null"],
                description: "Содержимое урока должно быть строкой или null"
              },
              position: {
                bsonType: ["int", "long"],
                minimum: 1,
                description: "Порядковый номер урока должен быть целым числом больше 0"
              },
              created_at: {
                bsonType: "date",
                description: "Дата создания урока должна иметь тип date"
              }
            }
          }
        }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});
