<template>
  <div class="index" top="0px">
    <canvas id="can"></canvas>
    <div class="intro">
      <div class="water-effect">
      
      <h1 class="fire-text">枪林弹雨</h1>
      <h1 id="ph5">Teamstyle20</h1>
      </div>
      
      <el-button @click="indexjump" v-if="flag" type="danger" size="medium" id="joinus">立即报名</el-button>
    </div>
    
  </div>
</template>
<script>
window.addEventListener("popstate", function(){
    //doSomething
    window.location = 'https://teamstyle.eesast.com';
}, false)

import nav from '@/components/nav.vue'
import { EventBus } from "../bus.js"
export default {
  name:"index",
  data (){
    return{
      flag : true
    }
  },
  methods:{
    indexjump(){
        EventBus.$emit('send-msg', {flag:true})
        this.$router.push({path: '/login'})
    }
  },
  computed:{
    show(){
      flag = nav.navflag;
    },
    
  },
  created:function()
  {
    window.onload=function(){


const canvas = document.getElementById("can");
const ctx = canvas.getContext("2d");
const points = [];
const fov = 100;
const dist = 100;
const opacity = 0.5;
const particleSize = 2;
const maxAmplitude = 1500; // Best results with values > 500
const sideLength = 50; // How many particles per side
const spacing = 200; // Particle distance from each other

let rotXCounter = 0;
let rotYCounter = 0;
let rotZCounter = 0;
let counter = 0;

canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

function Vector3(x, y, z) {
  this.x = x;
  this.y = y;
  this.z = z;
  this.color = "#0D0";
}

Vector3.prototype.rotateX = function(angle) {
  const z = this.z * Math.cos(angle) - this.x * Math.sin(angle);
  const x = this.z * Math.sin(angle) + this.x * Math.cos(angle);
  return new Vector3(x, this.y, z);
};

Vector3.prototype.rotateY = function(angle) {
  const y = this.y * Math.cos(angle) - this.z * Math.sin(angle);
  const z = this.y * Math.sin(angle) + this.z * Math.cos(angle);
  return new Vector3(this.x, y, z);
};
Vector3.prototype.rotateZ = function(angle) {
  const x = this.x * Math.cos(angle) - this.y * Math.sin(angle);
  const y = this.x * Math.sin(angle) + this.y * Math.cos(angle);
  return new Vector3(x, y, this.z);
};

Vector3.prototype.perspectiveProjection = function(fov, viewDistance) {
  const factor = fov / (viewDistance + this.z);
  const x = this.x * factor + canvas.width / 2;
  const y = this.y * factor + canvas.height / 2;
  return new Vector3(x, y, this.z);
};
Vector3.prototype.draw = function() {
  const frac = this.y / maxAmplitude;
  const r = Math.floor(frac * 100);
  const g = 20;
  const b = Math.floor(255 - frac * 100);
  const vec = this.rotateX(rotXCounter).rotateY(rotYCounter).rotateZ(rotZCounter).perspectiveProjection(fov, dist);

  this.color = `rgb(${r}, ${g}, ${b})`;
  ctx.fillStyle = this.color;
  ctx.fillRect(vec.x, vec.y, particleSize, particleSize);
};

// Init
for (let z = 0; z < sideLength; z++) {
  for (let x = 0; x < sideLength; x++) {
    const xStart = -(sideLength * spacing) / 2;
    points.push(
      new Vector3(xStart + x * spacing, 0, xStart + z * spacing)
    );
  }
}

function loop() {
  ctx.fillStyle = `rgba(0, 0, 0, ${opacity})`;
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  for (let i = 0, max = points.length; i < max; i++) {
    const x = i % sideLength;
    const z = Math.floor(i / sideLength);
    const xFinal = Math.sin(x / sideLength * 4 * Math.PI + counter);
    const zFinal = Math.cos(z / sideLength * 4 * Math.PI + counter);
    const gap = maxAmplitude * 0.3;
    const amp = maxAmplitude - gap;

    points[z * sideLength + x].y = maxAmplitude + xFinal * zFinal * amp;

    points[i].draw();
  }
  counter += 0.03;

  rotXCounter += 0.005;
  rotYCounter += 0.005;
  //rotZCounter += 0.005;

  window.requestAnimationFrame(loop);
};
loop();
  
}
}
}
// console.log(this.$route.path);

</script>

<style>
.index
{
  height:90%;
  min-height:75%;
  position: relative;
}
.el-carousel
{
  overflow: hidden;
}
.el-carousel__item{
}
#can 
{
  width:100%;
  height:100%;
}
.intro
{
    position: absolute;
    width:100%;
    height:40%;
    top:10%;
    color:azure;
}
.intro #ph4
{
  font-family: 'academy';
  font-size:100px;
  /* margin-top:0px; */
}
.intro #ph5
{
  font-family: 'Balonb'; 
  font-size:40px;
  /* margin-top:0px; */
}

.water-effect{
			text-align: center;
		}
.index		h1 {
	font-size: 120px;
  /* display: inline-block; */
  /* color:azure; */
	color: transparent;
	position: relative;
	background-clip: text;
	-webkit-background-clip: text;
	background-image: url('/static/img/water.gif');
	background-repeat: repeat-x;
	animation: watereffect 5s infinite normal linear;
	-webkit-text-stroke: 1px rgb(245, 62, 62);
}
h1.fire-text {
  top:50px;
	background-size: 70% 132%;
  background-image: url('/static/img/xx.gif');
  font-family: 'kuhei';
  font-weight: 100;
	-webkit-text-stroke: unset;
  animation:unset;
  background-position:-5% 160%;
}

	@keyframes watereffect{
		0%{background-position: 0 10px}
		5%{background-position: 5% 5px}
		10%{background-position: 10% -10px}
		15%{background-position: 15% -20px}
		20%{background-position: 20% -30px}
		25%{background-position: 25% -40px}
		30%{background-position: 30% -50px}
		40%{background-position:35% -55px}
		50%{background-position:40% -55px}
		60%{background-position:50% -55px}
		70%{background-position:60% -55px}
		80%{background-position:70% -55px}
		90%{background-position:80% -55px}
		100%{background-position:90% -55px}
	}
  /*.el-carousel__item h3 {
    color: #475669;
    font-size: 14px;
    opacity: 0.75;
    line-height: 150px;
    margin: 0;
  }
  .el-carousel__item:nth-child(2n) {
     background-color: #99a9bf;
  }
  
  .el-carousel__item:nth-child(2n+1) {
     background-color: #d3dce6;
  }*/


  @media screen and (max-width:720px) {
    .intro
    {
        position: absolute;
        width:100%;
        height:40%;
        top:40%;
        color:azure;
    }
    .intro #ph4
    {
      font-family: 'academy';
      font-size:40px;
      margin-top:10px;
    }
    .intro #ph5
    {
      font-family: 'Balonb'; 
      font-size:10vw;
      margin-top:0px;
      position: relative;
      top:-15vh;
    }
    h1.fire-text
    {
      font-size:20vw;
      position: relative;
      top:-10vh;
    }
    #joinus
    {
      position: relative;
      top:-15vh;
    }
  }
</style>