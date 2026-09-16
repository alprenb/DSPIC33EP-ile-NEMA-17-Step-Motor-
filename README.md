# dsPIC33EP ve TB6600 ile Açık Çevrim (Open-Loop) Redüktörlü Step Motor Kontrolü

Bu proje, **dsPIC33EP512MU814** mikrodenetleyicisi ve **TB6600HG** donanımsal motor sürücüsü kullanılarak 1/27 redüktörlü Nema 17 step motorun hız, yön ve tork kontrolünü sağlamaktadır. 

*Not: Bu repo, projeye kapalı çevrim (closed-loop) SSI enkoder geri beslemesi eklenmeden önceki **temel motor sürme ve donanımsal ısı yönetimi** altyapısını içermektedir.*

## 🛠 Donanım Bileşenleri

* **Mikrodenetleyici:** dsPIC33EP512MU814 (Fcy = 3.685 MHz)
* **Motor Sürücü:** TB6600HG HZIP-25 Analog Sürücü Entegresi
* **Motor:** 42HB34HJ-134 Nema 17 Redüktörlü Step Motor (Oran: 1/27, Tepe Akımı: 1.3A)

## 🔌 Pin Bağlantıları (Pinout)

Sistemin donanımsal izolasyonunu artırmak amacıyla sürücü tasarımında PGND ve SGND düzlemleri ayrılmış, lojik ve güç hatları dsPIC ile şu şekilde yapılandırılmıştır:

| dsPIC Pini | TB6600 Pini | Yön | İşlev |
| :--- | :--- | :--- | :--- |
| `RE2` | Pin 21 (CLK) | Çıkış | Adım (Pulse) Sinyali (Timer1 PWM) |
| `RA5` | Pin 22 (CW/CCW) | Çıkış | Motor Yön Sinyali (HIGH: Saat Yönü) |
| `RC15` | Pin 18 (ENABLE) | Çıkış | Çıkış Mosfetlerini Aktif/Pasif Etme |
| `RC12` | Pin 19 (RESET) | Çıkış | Sürücü Reset (Normal çalışma için HIGH) |
| `RC13` | Pin 3 (TQ) | Çıkış | Tork Kontrolü (Isı yönetimi için %30 akım modu) |
| `RC14` | Pin 1 (ALERT) | Giriş | Aşırı Akım/Isı Hata Sinyali (O.C. / T.S.D) |
| `RA4` | Pin 25 (MO) | Giriş | Başlangıç Fazı Monitör Sinyali |
| `RH12` | Pin 7 (M1) | Çıkış | Lojik Low (L) |
| `RA15` | Pin 8 (M2) | Çıkış | Lojik Low (L) |
| `RA14` | Pin 9 (M3) | Çıkış | Lojik High (H) - (Full-Step Modu Seçimi) |
| `RH13` | Pin 4 (LATCH)| Çıkış | Şeffaf/Kayıt Durumu Kontrolü |

## ⚙️ Sistem Mimarisi ve Teknik Detaylar

### 1. Timer1 ile Donanımsal Pulse (PWM) Üretimi
Motorun dönüş hızı ve hassas adım kontrolü, ana döngüyü (main loop) meşgul etmemek adına `Timer1` kesmesi (interrupt) ile arka planda yürütülmektedir.
* Timer çalışma frekansı (Prescaler 1:8) ile mikrodenetleyici üzerinde ~1000 Hz'lik düzenli bir donanımsal kesme oluşturulur.
* Sürücünün 1 tam adım atabilmesi için `RE2` (CLK) pininin 1 kez HIGH, 1 kez LOW olması (toggle) gerektiğinden, saniyede net **500 pulse (adım)** üretilir.
* Motorun iç devri saniyede 2.5 tur atarken, redüktör (1/27) sayesinde çıkış milinden yüksek torklu ve kararlı **~5.55 RPM** hız elde edilir.

### 2. Akım Sınırlandırma ve Aktif Isı Yönetimi (TQ Pin Algoritması)
TB6600HG entegresinin motor dururken sürekli maksimum tepe akımı (1.3A) çekip aşırı ısınmasını engellemek için çift katmanlı bir koruma uygulanmıştır:
* **Donanımsal Koruma:** Vref (Pin 5) bacağındaki referans voltaj, NFA/NFB pinlerindeki Rsense (Örn: 0.22 Ohm) şönt dirençlerine göre formülize edilerek donanımsal olarak (1.3A limitine) sabitlenmiştir.
* **Yazılımsal Tork Kontrolü:** Timer kesmesi içerisinde motor hareket halindeyken `TQ = 0` yapılarak sistemden %100 akım çekilmesi sağlanır. Motor istenilen adıma ulaşıp durduğunda `TQ = 1` komutu ile tutunma akımı (holding torque) %30 seviyesine düşürülür. Bu sayede motor fiziksel konumunu kaybetmezken ısı emisyonu drastik biçimde azaltılır.

### 3. 1/27 Redüktörlü Adım (Kinematik) Matematiği
Sistemdeki Nema 17 motorun doğal adımı 1.8 derecedir (1 tur = 200 adım).
* Redüktör oranı (1/27) göz önüne alındığında, çıkış milinin 1 tam tur (360 derece) dönmesi için gereken toplam adım sayısı: **200 x 27 = 5400 adım** (Full-Step modunda).
* Çıkış milinde tam 1 derecelik fiziksel hareket sağlamak için mikrokontrolörün **15 adım** (Pulse) üretmesi gerekmektedir (5400 / 360).

## 📂 Yazılım Yapısı (Modüler C Programlama)
Proje kodları, donanım birimlerine göre modüller halinde yapılandırılmıştır:
* `config.h`: İşlemci frekansı (FCY) makroları ve genel kütüphane bağımlılıkları.
* `motor.c / .h`: Timer1 kesme yapılandırması, pin atamaları, TQ ısı yönetim algoritması ve step motor donanım sürme fonksiyonları.
* `main.c`: Sistemi başlatan ve motorun hedef adım sayısına (hedef konuma) gitmesini tetikleyen ana döngü.

---
*Geliştirici:* Alp Eren
*Kurum:* Düzce Üniversitesi, Elektrik-Elektronik Mühendisliği
