<template>
    <div id="QR_contain">
        <!-- <img src="../../static/img/QRcode.jpg"></img> -->
        <h5><i class="el-icon-info"></i>本群仅用于接待，入群后请联系两位负责人：张钧（微信: ZJ-together）、饶淙元（微信：rcy-512），然后进入大群</h5>
        <img src="../../static/img/QRcode2.jpg"></img>
    </div>
</template>

<script>
export default {
    name:'QR',
    data(){
        return {

        }
    },
    methods:{
        
    }
}
</script>

<style>
#QR_contain{
    min-height:85%;
    margin-bottom: 5px;
    position: relative;
}
#QR_contain img{
    display: inline;
    width:auto;
    height:90vh;
    position: relative;
}
@media screen and (max-width: 720px)
 { 
     #QR_contain{
    min-height:90vh;
    margin-bottom: 5px;
    }
    #QR_contain h5
    {
        position: relative;
        top:10vh;
    }
     #QR_contain img{
    width:auto;
    max-height:78vh;
    position: relative;
    top:10vh;
    }
 }

</style>
