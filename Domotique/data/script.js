var url="";
var mois = new Date().getMonth();

function CreerGraph(balise,label,data){

const ctx = document.getElementById(balise).children[0].getContext('2d');
const myChart = new Chart(ctx, {
    type: 'bar',
    data: {
        labels: ['Janvier', 'Février', 'Mars', 'Avril', 'Mai', 'Juin','Juillet','Août','Septembre','Octobre','Novembre','Décembre'],
        datasets: [{
            label: label,
            data: data,
            backgroundColor: [
                'rgba(255, 99, 132, 0.2)',
                'rgba(54, 162, 235, 0.2)',
                'rgba(255, 206, 86, 0.2)',
                'rgba(75, 192, 192, 0.2)',
                'rgba(153, 102, 255, 0.2)',
                'rgba(255, 159, 64, 0.2)',
                'rgba(200, 69, 164, 0.2)',
                'rgba(255, 99, 132, 0.2)',
                'rgba(54, 162, 235, 0.2)',
                'rgba(255, 206, 86, 0.2)',
                'rgba(75, 192, 192, 0.2)',
                'rgba(153, 102, 255, 0.2)'
            ],
            borderColor: [
                'rgba(255, 99, 132, 1)',
                'rgba(54, 162, 235, 1)',
                'rgba(255, 206, 86, 1)',
                'rgba(75, 192, 192, 1)',
                'rgba(153, 102, 255, 1)',
                'rgba(255, 159, 64, 1)',
                'rgba(200, 69, 164, 1)',
                'rgba(255, 99, 132, 1)',
                'rgba(54, 162, 235, 1)',
                'rgba(255, 206, 86, 1)',
                'rgba(75, 192, 192, 1)',
                'rgba(153, 102, 255, 1)'
            ],
            borderWidth: 1
        }]
    },
            options: {
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: true
                    }
                }
            }
});
}

function RecupererHeure(){
	var Horaire = new Date();
	var heure   = Horaire.getHours();
	var minute  = Horaire.getMinutes();
	var seconde = Horaire.getSeconds();
	if(minute<10){
		minute="0"+minute
	}

	if(seconde<10){
		seconde="0"+seconde
	}

	$("main p").text(`${heure}:${minute}:${seconde}`)
}

function RecupererDate() {
	var date = new Date();
	var options = {weekday: "long", year: "numeric", month: "long", day: "2-digit"};
	$("nav").text(date.toLocaleDateString("fr-FR", options))
}
function FaireApparaitre(balise,temps){
	$(balise).hide();
	$(balise).fadeIn(temps);
}

function RecupererTemperature(){
			$.ajax({
			method: "GET",
			url: url+"Temperature",
		})
		.done(function( msg ) {
			$("#temperature").text(`Température actuelle : ${msg}°C`)
		});
}

function RecupererPuissanceinstantanee(){
	$.ajax({
		method: "GET",
		url: url+"Puissanceinstantanee",
	})
		.done(function( msg ) {
			$("#PI").text(`Puissance Instantanée :${msg}`)
		});
}

var consoEau=[];

function RecupererConsoEau(){
			$.ajax({
			method: "GET",
			url: url+"ConsoEau",
		})
		.done(function( msg ) {
			if(consoEau[mois]!=msg.split(',')[mois]){
			consoEau=msg.split(',');
			$("#content1").empty();
			$("#content1").append('<canvas width="250" height="250">');
			CreerGraph("content1","Consommation d'eau en litres",consoEau);
			$("#content3").empty();
			$("#content3").append('<canvas width="250" height="250">');
			var canvas = document.getElementById("content3").children[0];
			var ctx = canvas.getContext("2d");
			ctx.font = "29px Arial";
			ctx.textAlign = "center";
			ctx.fillStyle = "grey";
			ctx.fillText((consoEau[mois]*0.00489).toFixed(2)+" € d'eau", canvas.width/2, canvas.height/2);
			}
		});
}

var consoElec=[];
var ancienTableau=[];
var nouveauTableau=[];

function RecupererConsoElec(){
			$.ajax({
			method: "GET",
			url: url+"ConsoElec",
		})
		.done(function( msg ) {
			for(let i=0;i<msg.length;i++){
				consoElec[i]=Math.round((msg.split(',')[i])/1000);
				nouveauTableau[i]=Math.round((msg.split(',')[i])/1000);
			}
			if(ancienTableau!=nouveauTableau){
			ancienTableau=nouveauTableau;
			$("#content2").empty();
			$("#content2").append('<canvas width="250" height="250">');
			CreerGraph("content2","Consommation d'électricité en KiloWatt",consoElec);
			$("#content4").empty();
			$("#content4").append('<canvas width="250" height="250">');
			var canvas = document.getElementById("content4").children[0];
			var ctx = canvas.getContext("2d");
			ctx.font = "29px Arial";
			ctx.textAlign = "center";
			ctx.fillStyle = "grey";
			ctx.fillText((consoElec[mois]*0.198).toFixed(2)+" € d'électricité", canvas.width/2, canvas.height/2);
			}
		});
}

function RecupererEtatsRelais(){
		$.ajax({
            type: "GET",
            url: url+"EtatRelais",
        })
			.done(function( msg ) {
				console.log();
				let tableauEtats=msg.split(",");
				for(let i=0;i<tableauEtats.length-1;i++){
					if(tableauEtats[i]==1){
						$($("div input")[i]).attr('checked',false)
						$($("div img")[i]).attr('src', "LedEteinte.png")
					}
					else if (tableauEtats[i]==0){
						$($("div input")[i]).attr('checked',true)
						$($("div img")[i]).attr('src', "LedAllume.png")
					}
				}
				$($("div input")[2]).attr('disabled',true)
				$($("div img")[2]).attr('src', "LedHS.png")
				
		});
}

function ActiverRelai(Bouton){
		let image=Bouton.parentNode.nextElementSibling;
		let IDSwitch=Bouton.parentNode.parentNode.id;
		
		
		switch (IDSwitch) {
			case 'switch1':NumRelai=1;break;
			case 'switch2':NumRelai=2;break;
			case 'switch3':NumRelai=3;break;
			case 'switch4':NumRelai=4;break;
			case 'switch5':NumRelai=5;break;
			case 'switch6':NumRelai=6;break;
			case 'switch7':NumRelai=7;break;
			case 'switch8':NumRelai=8;break;
		}
		$.ajax({
			method: "GET",
			url: url+"on",
			data: { Relai: NumRelai}
		})
		.done(function( msg ) {
			if(msg==0)
				$(image).attr('src', "LedAllume.png")
			else if (msg=="Impulsion")
			{
				$(image).attr('src', "LedAllume.png")
				setTimeout(() => {Bouton.checked=false;$(image).attr('src', "LedEteinte.png")}, 1000);
			}
			else
				$(image).attr('src', "LedEteinte.png")
		});
}

function RecupererJourNuit(){
	let image=$("#JourNuit").children('img')[0];
			$.ajax({
			method: "GET",
			url: url+"JourNuit"
		})
		.done(function( msg ) {
			if(msg==0){
				$(image).attr('src', "moon.png")
				$($("div input")[2]).attr('disabled',false)
				$($("div img")[2]).attr('src',"LedEteinte.png")
			}
			else{
				$(image).attr('src', "sun.png")
				$($("div input")[2]).attr('disabled',true)
				$($("div img")[2]).attr('src',"LedHS.png")
			}
		});
}

$(document).ready(function(){
	$("#temperature").text(`Température actuelle : 30°C`)
	$("#PI").text(`Puissance Instantanée : 450w`)

	RecupererDate();
	RecupererHeure();
	RecupererTemperature();
	RecupererConsoEau();
	RecupererConsoElec();
	RecupererEtatsRelais();
	RecupererJourNuit();
	RecupererPuissanceinstantanee()

	setInterval(RecupererHeure,1000);
	setInterval(RecupererTemperature,60000);
	setInterval(RecupererConsoEau,10000);
	setInterval(RecupererConsoElec,11000);
	setInterval(RecupererJourNuit,12000);
	setInterval(RecupererPuissanceinstantanee,13000);

	FaireApparaitre("#sidebar",1000);
	FaireApparaitre("nav",1000);
	FaireApparaitre("main",1200);
	FaireApparaitre("#content1",1400);
	FaireApparaitre("#content2",1600);
	FaireApparaitre("#content3",1800);
	FaireApparaitre("#content4",2000);

	$("input").change(function(){ActiverRelai(this)});
})