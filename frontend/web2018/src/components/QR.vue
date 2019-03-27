<template>
    <div id="QR_contain">
        <img src="../../static/img/QRcode.jpg"></img>
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
     #QR_contain img{
    width:auto;
    min-height:78%;
    position: relative;
    top:15%;
}
 }

</style>
