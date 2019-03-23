<template>
    <div id="QR_contain">
        <img src="../../static/img/QRcode.jpg"></img>
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
    height:80%;
    margin-bottom: 5px;
}
#QR_contain img{
    width:auto;
    height:100%;
}
@media screen and (max-width: 720px)
 { 
     #QR_contain{
    height:90vh;
    margin-bottom: 5px;
    }
     #QR_contain img{
    width:auto;
    height:78%;
    position: relative;
    top:15%;
}
 }

</style>
