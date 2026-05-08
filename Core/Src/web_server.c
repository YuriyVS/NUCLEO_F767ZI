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
// Буфер для входящих данных от SPA (хватит для пачки параметров)
static char post_rx_buffer[4096];
static uint16_t post_rx_len = 0;

AlarmEvent_t alarm_history[ALARM_LOG_SIZE];
uint16_t alarm_head = 0; // Указатель на текущую позицию
uint32_t last_fault_mask = 0; // Состояние аварий в прошлом цикле

// Весь ваш сайт в одной переменной
//const char index_html_content[] =
//"<!DOCTYPE html>"
//"<html>"
//"<head><meta charset=\"UTF-8\"><title>NUCLEO F767ZI</title>"
//"<style>"
//"body { font-family:sans-serif; background:#2c3e50; color:white; padding:20px; }"
//".card { background:#34495e; padding:15px; border-radius:10px; margin:10px; border-left:5px solid #2ecc71; }"
//".val { font-size:24px; font-weight:bold; color:#2ecc71; }"
//"</style></head>"
//"<body>"
//"<h1>Параметры системы</h1>"
//"<div class=\"card\"><div>Напряжение сети:</div><div id=\"volt\" class=\"val\">--</div></div>"
//"<div class=\"card\"><div>Ток АКБ:</div><div id=\"freq\" class=\"val\">--</div></div>"
//"<script>"
//"async function update(){"
//"  try {"
//"    const res = await fetch('/api/main');"
//"    const data = await res.json();"
//"    document.getElementById('volt').innerText = data.UsetiV.toFixed(1) + ' V';"
//"    document.getElementById('freq').innerText = data.IakbA.toFixed(1) + ' А';"
//"  } catch(e){}"
//"}"
//"setInterval(update, 500);"
//"</script>"
//"</body></html>";

//const char index_html_content[] =
//"<!DOCTYPE html><html lang=\"ru\"><head><meta charset=\"UTF-8\">"
//"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
//"<title>NUCLEO F767ZI | Control Panel</title>"
//"<style>"
//"body{font-family:segoe ui,arial;background:#1a1a1a;color:#eee;margin:0;padding:20px}"
//".tabs{display:flex;margin-bottom:20px;border-bottom:2px solid #333}"
//".tab{padding:10px 20px;cursor:pointer;border:1px solid transparent;margin-bottom:-2px}"
//".tab.active{border:2px solid #2ecc71;border-bottom-color:#1a1a1a;color:#2ecc71;font-weight:bold}"
//".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:15px}"
//".card{background:#252525;padding:15px;border-radius:5px;box-shadow:0 2px 5px rgba(0,0,0,0.5);border-left:4px solid #3498db}"
//".card.param{border-left-color:#e67e22}"
//".lab{font-size:12px;color:#888;text-transform:uppercase;margin-bottom:5px}"
//".val{font-size:22px;font-weight:bold;color:#2ecc71}"
//"input{background:#333;border:1px solid #444;color:#fff;padding:5px;width:100%;font-size:18px;border-radius:3px}"
//"button{background:#2ecc71;border:none;padding:10px 20px;color:#fff;cursor:pointer;margin-top:20px;border-radius:3px}"
//"button:hover{background:#27ae60}"
//"</style></head><body>"
//"<h1>Система управления Nucleo-F7</h1>"
//"<div class=\"tabs\"><div id=\"t1\" class=\"tab active\" onclick=\"setTab(1)\">Мониторинг</div>"
//"<div id=\"t2\" class=\"tab\" onclick=\"setTab(2)\">Параметры</div></div>"
//"<div id=\"view1\" class=\"grid\"></div>"
//"<div id=\"view2\" class=\"grid\" style=\"display:none\"></div>"
//"<button id=\"saveBtn\" style=\"display:none\" onclick=\"save()\">Сохранить параметры</button>"
//"<script>"
//"let currentTab = 1;"
//"function setTab(t){"
//"  currentTab=t;document.getElementById('t1').className=t==1?'tab active':'tab';"
//"  document.getElementById('t2').className=t==2?'tab active':'tab';"
//"  document.getElementById('view1').style.display=t==1?'grid':'none';"
//"  document.getElementById('view2').style.display=t==2?'grid':'none';"
//"  document.getElementById('saveBtn').style.display=t==2?'block':'none';"
//"  update();"
//"}"
//"async function update(){"
//"  const url = currentTab==1 ? '/api/main' : '/api/params';"
//"  try {"
//"    const res = await fetch(url); const data = await res.json();"
//"    const container = document.getElementById('view'+currentTab);"
//"    for(let key in data){"
//"      let el = document.getElementById('v_'+key);"
//"      if(!el){"
//"        const card = document.createElement('div'); card.className=currentTab==1?'card':'card param';"
//"        card.innerHTML = `<div class='lab'>${key}</div>`;"
//"        if(currentTab==1) card.innerHTML += `<div id='v_${key}' class='val'>--</div>`;"
//"        else card.innerHTML += `<input id='v_${key}' type='number' step='0.1' value='${data[key]}'>`;"
//"        container.appendChild(card);"
//"      } else if(currentTab==1) el.innerText = data[key];"
//"    }"
//"  } catch(e){}"
//"}"
//"async function save(){"
//"  const inputs = document.querySelectorAll('#view2 input'); let payload={};"
//"  inputs.forEach(i => payload[i.id.replace('v_','')] = parseFloat(i.value));"
//"  const res = await fetch('/api/save', {method:'POST', body:JSON.stringify(payload)});"
//"  if(res.ok) alert('Параметры отправлены!');"
//"}"
//"setInterval(()=>{ if(currentTab==1) update(); }, 1000);"
//"update();"
//"</script></body></html>";

const char index_html_content[] =
"<!DOCTYPE html><html lang=\"ru\"><head><meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
"<title>NUCLEO F767ZI | Control Panel</title>"
"<style>"
"body{font-family:segoe ui,arial;background:#1a1a1a;color:#eee;margin:0;padding:20px}"
".tabs{display:flex;margin-bottom:20px;border-bottom:2px solid #333;overflow-x:auto}"
".tab{padding:10px 20px;cursor:pointer;white-space:nowrap;border:1px solid transparent;margin-bottom:-2px;color:#888}"
".tab.active{border:2px solid #2ecc71;border-bottom-color:#1a1a1a;color:#2ecc71;font-weight:bold}"
".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:15px}"
".card{background:#252525;padding:15px;border-radius:5px;box-shadow:0 2px 5px rgba(0,0,0,0.5);border-left:4px solid #3498db}"
".card.param{border-left-color:#e67e22}"
".card.alarm{border-left-color:#e74c3c;animation:blink 2s infinite}"
"@keyframes blink{0%{opacity:1}50%{opacity:0.7}100%{opacity:1}}"
".lab{font-size:12px;color:#888;text-transform:uppercase;margin-bottom:5px}"
".val{font-size:22px;font-weight:bold;color:#2ecc71}"
"table{width:100%;border-collapse:collapse;background:#252525;border-radius:5px;overflow:hidden}"
"th,td{padding:12px;text-align:left;border-bottom:1px solid #333}"
"th{background:#333;color:#888;font-size:12px;text-transform:uppercase}"
"input{background:#333;border:1px solid #444;color:#fff;padding:5px;width:100%;font-size:18px;border-radius:3px}"
"button{background:#2ecc71;border:none;padding:10px 20px;color:#fff;cursor:pointer;margin-top:20px;border-radius:3px}"
"button:hover{background:#27ae60}"
"</style></head><body>"
"<h1>Система управления Nucleo-F7</h1>"
"<div class=\"tabs\">"
"<div id=\"t1\" class=\"tab active\" onclick=\"setTab(1)\">Мониторинг</div>"
"<div id=\"t2\" class=\"tab\" onclick=\"setTab(2)\">Параметры</div>"
"<div id=\"t3\" class=\"tab\" onclick=\"setTab(3)\">Активные</div>"
"<div id=\"t4\" class=\"tab\" onclick=\"setTab(4)\">Журнал</div>"
"</div>"
"<div id=\"view1\" class=\"grid\"></div>"
"<div id=\"view2\" class=\"grid\" style=\"display:none\"></div>"
"<div id=\"view3\" class=\"grid\" style=\"display:none\"></div>"
"<div id=\"view4\" style=\"display:none\">"
"<table><thead><tr><th>Время (сек)</th><th>Событие</th><th>Статус</th></tr></thead><tbody id=\"logBody\"></tbody></table>"
"</div>"
"<button id=\"saveBtn\" style=\"display:none\" onclick=\"save()\">Сохранить параметры</button>"
"<script>"
"let currentTab = 1;"
"const ALARM_NAMES = {"
"  0: 'Низкое давление масла',"
"  1: 'Высокая температура ОЖ',"
"  2: 'Остановка по перегрузке',"
"  3: 'Авария заземления',"
"  4: 'Низкий уровень топлива'"
"  /* Добавьте остальные до 63 */"
"};"
"function setTab(t){"
"  currentTab=t;"
"  for(let i=1;i<=4;i++){"
"    document.getElementById('t'+i).className = (i==t)?'tab active':'tab';"
"    document.getElementById('view'+i).style.display = (i==t)?(i==4?'block':'grid'):'none';"
"  }"
"  document.getElementById('saveBtn').style.display = (t==2)?'block':'none';"
"  update();"
"}"
"async function update(){"
"  let url = '/api/main';"
"  if(currentTab==2) url = '/api/params';"
"  if(currentTab==4) url = '/api/history';"
"  try {"
"    const res = await fetch(url); const data = await res.json();"
"    const container = document.getElementById('view'+currentTab);"
"    if(currentTab <= 2){"
"      for(let key in data){"
"        let el = document.getElementById('v_'+key);"
"        if(!el){"
"          const card = document.createElement('div'); card.className=currentTab==1?'card':'card param';"
"          card.innerHTML = `<div class='lab'>${key}</div>`;"
"          if(currentTab==1) card.innerHTML += `<div id='v_${key}' class='val'>--</div>`;"
"          else card.innerHTML += `<input id='v_${key}' type='number' step='0.1' value='${data[key]}'>`;"
"          container.appendChild(card);"
"        } else if(currentTab==1) el.innerText = data[key];"
"      }"
"    } else if(currentTab==3){"
"      container.innerHTML = '';"
"      const mask = BigInt(data.Fault || 0);"
"      for(let i=0;i<64;i++){"
"        if((mask >> BigInt(i)) & 1n){"
"          container.innerHTML += `<div class='card alarm'><div class='lab'>ID: ${i}</div><div class='val' style='color:#e74c3c'>${ALARM_NAMES[i]||'Неизвестная ошибка'}</div></div>`;"
"        }"
"      }"
"      if(!container.innerHTML) container.innerHTML = '<p>Активных аварий нет</p>';"
"    } else if(currentTab==4){"
"      const body = document.getElementById('logBody');"
"      body.innerHTML = '';"
"      data.sort((a,b)=>b.t-a.t).forEach(e => {"
"        body.innerHTML += `<tr><td>${e.t}</td><td>${ALARM_NAMES[e.id]||'Ошибка '+e.id}</td>` +"
"          `<td style='color:${e.s?\"#e74c3c\":\"#2ecc71\"}'>${e.s?'ВОЗНИКЛА':'УШЛА'}</td></tr>`;"
"      });"
"    }"
"  } catch(e){}"
"}"
"async function save(){"
"  const inputs = document.querySelectorAll('#view2 input'); let payload={};"
"  inputs.forEach(i => payload[i.id.replace('v_','')] = parseFloat(i.value));"
"  const res = await fetch('/api/save', {method:'POST', body:JSON.stringify(payload)});"
"  if(res.ok) alert('Параметры сохранены!');"
"}"
"setInterval(()=>{ if(currentTab!=2) update(); }, 1000);"
"update();"
"</script></body></html>";

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
            "{\"UsetiV\":%.1f,\"IakbA\":%.1f,\"GenFreqHz\":%.1f,\"Fault\":%lu}",
            DBMain.f50.UsetiV,
            DBMain.f50.IakbA,
            DBMain.f50.GenFreqHz,
            DBMain.b64.all); // Добавьте нужные поля по аналогии

//    	int len = snprintf(web_json_buffer, sizeof(web_json_buffer),
//    	    "{\"UsetiV\":%.1f,\"IakbA\":%.1f,\"GenFreqHz\":%.1f,\"Fault\":%llu}",
//    	    DBMain.f50.UsetiV,
//    	    DBMain.f50.IakbA,
//    	    DBMain.f50.GenFreqHz,
//    	    (unsigned long long)DBMain.b64.all); // Используем %llu для 64 бит

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
//    	int offset = snprintf(web_json_buffer, sizeof(web_json_buffer), "{");
//
//    	    // Указатель на начало массива float в вашей структуре
//    	    float *f_ptr = (float *)&DBParameters.f50;
//
//    	    for (int i = 0; i < 50; i++) {
//    	        offset += snprintf(web_json_buffer + offset, sizeof(web_json_buffer) - offset,
//    	                           "\"P%d\":%.2f%s", i+1, f_ptr[i], (i == 49) ? "" : ",");
//    	    }
//
//    	    snprintf(web_json_buffer + offset, sizeof(web_json_buffer) - offset, "}");
//
//    	    file->data = web_json_buffer;
//    	    file->len = strlen(web_json_buffer);
//    	    file->index = file->len;
//    	    file->pextension = NULL;
//    	    return 1;

    }

    if (strcmp(name, "/api/history") == 0) {
        int offset = snprintf(web_json_buffer, sizeof(web_json_buffer), "[");

        for (int i = 0; i < ALARM_LOG_SIZE; i++) {
            // Пропускаем пустые записи (где время 0)
            if (alarm_history[i].uptime == 0) continue;

            offset += snprintf(web_json_buffer + offset, sizeof(web_json_buffer) - offset,
                "{\"t\":%lu,\"id\":%d,\"s\":%d}%s",
                alarm_history[i].uptime,
                alarm_history[i].alarm_id,
                alarm_history[i].is_active,
                (i == ALARM_LOG_SIZE - 1) ? "" : ",");
        }

        // Убираем лишнюю запятую в конце, если она есть, и закрываем массив
        if (web_json_buffer[offset-1] == ',') offset--;
        snprintf(web_json_buffer + offset, sizeof(web_json_buffer) - offset, "]");

        file->data = web_json_buffer;
        file->len = strlen(web_json_buffer);
        file->index = file->len;
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
    //snprintf(response_uri, response_uri_len, "/index.html");
	if (strcmp(uri, "/api/save") == 0) {
	        post_rx_len = 0; // Сбрасываем буфер перед приемом
	        return ERR_OK;
	    }
    return ERR_OK;
}

// Вызывается при получении каждой порции данных POST
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
    // Здесь мы будем парсить JSON, когда будем сохранять DBParameters
    if (p != NULL) {
    	// Копируем данные из pbuf в наш статический буфер
    	        uint16_t len = pbuf_copy_partial(p, &post_rx_buffer[post_rx_len], p->tot_len, 0);
    	        post_rx_len += len;
    	        pbuf_free(p); // Обязательно освобождаем память пакета!
    }
    return ERR_OK;
}

// Вызывается, когда все данные POST получены
void httpd_post_finished(void *connection, char *response_uri, uint16_t response_uri_len)
{
		post_rx_buffer[post_rx_len] = '\0'; // Завершаем строку

	    // ПРИМЕР ручного парсинга JSON "на коленке"
	    // Ищем строку "P1_1": и забираем число после нее
	    char *ptr = strstr(post_rx_buffer, "\"P1_1\":");
	    if (ptr) {
	        DBParameters.f50.P1_1 = strtof(ptr + 7, NULL);
	    }

	    ptr = strstr(post_rx_buffer, "\"P1_2\":");
	    if (ptr) {
	        DBParameters.f50.P1_2 = strtof(ptr + 7, NULL);
	    }

	    // После обработки отправляем пользователя обратно (или отдаем успех)
	    strncpy(response_uri, "/index.html", response_uri_len);

	    // ВАЖНО: Если у вас есть функция сохранения во Flash, вызывайте её здесь!
	    // SaveParamsToFlash();
}

// --- ОБРАБОТКА CGI (если включено в CubeMX) ---

// Если в логах есть ошибка по httpd_cgi_handler, добавьте это:
// Правильная сигнатура для вашей версии LwIP
void httpd_cgi_handler(struct fs_file *file, const char* uri, int iNumParams, char **pcParam, char **pcValue)
{
    // Пока оставляем пустой заглушкой.
    // Поскольку мы используем SPA, этот обработчик нам не понадобится.
}

void Process_Alarm_Log(void) {
    uint32_t current_faults = DBMain.b64.all;
    uint32_t changed_bits = current_faults ^ last_fault_mask; // Находим изменившиеся биты

    if (changed_bits != 0) {
        uint32_t current_time = HAL_GetTick() / 1000;

        for (int i = 0; i < 32; i++) {
            if ((changed_bits >> i) & 1) { // Если i-й бит изменился
                // Записываем событие в историю
                alarm_history[alarm_head].uptime = current_time;
                alarm_history[alarm_head].alarm_id = i;
                alarm_history[alarm_head].is_active = (current_faults >> i) & 1;

                // Сдвигаем голову кольцевого буфера
                alarm_head = (alarm_head + 1) % ALARM_LOG_SIZE;
            }
        }
    }
    last_fault_mask = current_faults;
}
