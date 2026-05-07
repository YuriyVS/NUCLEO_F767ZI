#include "lwip/apps/fs.h" // КРИТИЧНО: здесь определение struct fs_file
#include "web_server.h"
#include "DB_Main.h"
#include "DB_Parameters.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwip/mem.h"

// Буфер для формирования JSON.
// ВНИМАНИЕ: Для 50+ float значений размер должен быть около 2048-4096 байт
char web_json_buffer[4096];

// Весь ваш сайт в одной переменной
const char index_html_content[] =
"<!DOCTYPE html>"
"<html>"
"<head><meta charset=\"UTF-8\"><title>NUCLEO F767ZI</title>"
"<style>"
"body { font-family:sans-serif; background:#2c3e50; color:white; padding:20px; }"
".card { background:#34495e; padding:15px; border-radius:10px; margin:10px; border-left:5px solid #2ecc71; }"
".val { font-size:24px; font-weight:bold; color:#2ecc71; }"
"</style></head>"
"<body>"
"<h1>Параметры системы</h1>"
"<div class=\"card\"><div>Напряжение сети:</div><div id=\"volt\" class=\"val\">--</div></div>"
"<div class=\"card\"><div>Ток АКБ:</div><div id=\"freq\" class=\"val\">--</div></div>"
"<script>"
"async function update(){"
"  try {"
"    const res = await fetch('/api/main');"
"    const data = await res.json();"
"    document.getElementById('volt').innerText = data.UsetiV.toFixed(1) + ' V';"
"    document.getElementById('freq').innerText = data.IakbA.toFixed(1) + ' А';"
"  } catch(e){}"
"}"
"setInterval(update, 500);"
"</script>"
"</body></html>";

int fs_open_custom(struct fs_file *file, const char *name)
{
	// 1. Если просят главную страницу (корень или index.html)
	    if (strcmp(name, "/") == 0 || strcmp(name, "/index.html") == 0) {
	        file->data = (const char *)index_html_content;
	        file->len = strlen(index_html_content);
	        file->index = file->len;
	        file->pextension = NULL;
	        return 1;
	    }

	// 1. Проверяем, какой именно API-запрос пришел
    if (strcmp(name, "/api/main") == 0) {

        // Формируем JSON из структуры DBMain
        // Используем snprintf для безопасности
        int len = snprintf(web_json_buffer, sizeof(web_json_buffer),
            "{\"UsetiV\":%.1f,\"IakbA\":%.1f,\"GenFreqHz\":%.1f,\"Fault\":%d}",
            DBMain.f50.UsetiV,
            DBMain.f50.IakbA,
            DBMain.f50.GenFreqHz,
            DBMain.b64.FaultGenerator); // Добавьте нужные поля по аналогии

        // Заполняем структуру файла для LwIP
        file->data = web_json_buffer;
        file->len = len;
        file->index = len;
        file->pextension = NULL;
        return 1; // Файл "открыт" успешно
    }

    if (strcmp(name, "/api/params") == 0) {
        // Аналогично формируем JSON для DBParameters
        int len = snprintf(web_json_buffer, sizeof(web_json_buffer),
            "{\"P1_1\":%.2f,\"P1_2\":%.2f}",
            DBParameters.f50.P1_1,
            DBParameters.f50.P1_2);

        file->data = web_json_buffer;
        file->len = len;
        file->index = len;
        file->pextension = NULL;
        return 1;
    }

    return 0; // Для всех остальных файлов (index.html, JS, CSS) возвращаем 0
}

void fs_close_custom(struct fs_file *file) {
    // Здесь можно освободить ресурсы, если вы использовали malloc
    // В нашем случае с статическим буфером делать ничего не нужно
}

// --- ОБРАБОТКА POST ЗАПРОСОВ ---

// Вызывается в момент начала получения POST-запроса
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       uint16_t http_request_len, int content_len, char *response_uri,
                       uint16_t response_uri_len, uint8_t *post_auto_wnd)
{
    // Пока просто принимаем все POST запросы и отправляем на index.html
    snprintf(response_uri, response_uri_len, "/index.html");
    return ERR_OK;
}

// Вызывается при получении каждой порции данных POST
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
    // Здесь мы будем парсить JSON, когда будем сохранять DBParameters
    if (p != NULL) {
        pbuf_free(p); // Обязательно освобождаем буфер
    }
    return ERR_OK;
}

// Вызывается, когда все данные POST получены
void httpd_post_finished(void *connection, char *response_uri, uint16_t response_uri_len)
{
    // Можно сменить страницу ответа здесь
}

// --- ОБРАБОТКА CGI (если включено в CubeMX) ---

// Если в логах есть ошибка по httpd_cgi_handler, добавьте это:
// Правильная сигнатура для вашей версии LwIP
void httpd_cgi_handler(struct fs_file *file, const char* uri, int iNumParams, char **pcParam, char **pcValue)
{
    // Пока оставляем пустой заглушкой.
    // Поскольку мы используем SPA, этот обработчик нам не понадобится.
}
